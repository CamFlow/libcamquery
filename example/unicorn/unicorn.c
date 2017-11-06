/*
 *
 * Author: Thomas Pasquier <tfjmp@g.harvard.edu>
 * Author: Xueyuan Han <hanx@g.harvard.edu>
 *
 * Copyright (C) 2017 Harvard University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 */
#define SERVICE_QUERY
#include <camquery.h>

#define pwrtwo(x) (1 << (x))

#include "vector.h"

#define MAX_PARENT 2

FILE *logfile;

static void init( void ){
  print("unicornd ready!");
  init_files();
}

static void print_node(prov_entry_t* node){
  switch(node_type(node)){
    case ACT_TASK:
      write_task(node);
      break;
    case ENT_INODE_FILE:
      write_file(node);
      break;
    default:
      break;
  }
}

void* zalloc(size_t size){
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

#define task_propagate_value(param) ep->param[0] = node->task_info.param; memcpy(&(ep->param[1]), np->param, (MAX_DEPTH-1)*sizeof(uint64_t))
#define mode_propagate_value(param) ep->param[0] = node->inode_info.param; memcpy(&(ep->param[1]), np->param, (MAX_DEPTH-1)*sizeof(uint16_t))

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  struct vec *ep;
  struct vec *np;

  if (edge->msg_info.var_ptr == NULL)
    edge->msg_info.var_ptr = zalloc(sizeof(struct vec));
  ep = edge->msg_info.var_ptr;

  if (node->msg_info.var_ptr == NULL)
    node->msg_info.var_ptr = zalloc(sizeof(struct vec));
  np = node->msg_info.var_ptr;

  if (edge_type(edge) == RL_VERSION_PROCESS) {
    task_propagate_value(utime);
    task_propagate_value(stime);
    task_propagate_value(vm);
    task_propagate_value(rss);
    task_propagate_value(hw_vm);
    task_propagate_value(hw_rss);
    task_propagate_value(rbytes);
    task_propagate_value(wbytes);
    task_propagate_value(cancel_wbytes);
    task_propagate_value(utsns);
    task_propagate_value(ipcns);
    task_propagate_value(mntns);
    task_propagate_value(pidns);
    task_propagate_value(netns);
    task_propagate_value(cgroupns);
  } else if(edge_type(edge) == RL_VERSION && prov_is_inode(node_type(node))) {
    mode_propagate_value(mode);
  }
  ep->in_node = node_type(node);
  memcpy(ep->p_type, np->p_type, REC_SIZE*sizeof(uint64_t));
  memcpy(ep->in_type, np->in_type, REC_SIZE*sizeof(uint64_t));
  if(prov_has_uidgid(node_type(node)))
    ep->uid = node->msg_info.uid;
  else
    ep->uid = -2;
  memcpy(ep->p_uid, np->p_uid, REC_SIZE*sizeof(uint64_t));
  if(prov_has_uidgid(node_type(node)))
    ep->gid = node->msg_info.uid;
  else
    ep->gid = -2;
  memcpy(ep->p_gid, np->p_gid, REC_SIZE*sizeof(uint64_t));
  print_node(node);
  return 0;
}

#define task_retrieve_value(param) memcpy(np->param, ep->param, MAX_DEPTH*sizeof(uint64_t))
#define mode_retrieve_value(param) memcpy(np->param, ep->param, MAX_DEPTH*sizeof(uint16_t))
#define task_retrieve_ancestry(param) for(i=1; i<MAX_DEPTH; i++){\
                                        pos = CALC(i) + np->in*pwrtwo(i);\
                                        nb = pwrtwo(i);\
                                        pos2 = CALC(i-1);\
                                        for(j=0;j<nb;j++){\
                                          np->param[pos+j]=ep->param[pos2+j];\
                                        }\
                                      }

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  struct vec *ep;
  struct vec *np;
  int i, j;
  int nb, pos, pos2;

  if (edge->msg_info.var_ptr == NULL)
    edge->msg_info.var_ptr = zalloc(sizeof(struct vec));
  ep = edge->msg_info.var_ptr;

  if (node->msg_info.var_ptr == NULL)
    node->msg_info.var_ptr = zalloc(sizeof(struct vec));
  np = node->msg_info.var_ptr;

  /* couting in edges */

  if (np->in < MAX_PARENT) {
    np->in_type[np->in] = edge_type(edge);
    task_retrieve_ancestry(in_type);
    np->p_type[np->in] = ep->in_node;
    task_retrieve_ancestry(p_type);
    np->p_uid[np->in] = ep->uid;
    task_retrieve_ancestry(p_uid);
    np->p_gid[np->in] = ep->gid;
    task_retrieve_ancestry(p_gid);
  }
  np->in++;

  if (edge_type(edge) == RL_VERSION_PROCESS) {
    task_retrieve_value(utime);
    task_retrieve_value(stime);
    task_retrieve_value(vm);
    task_retrieve_value(rss);
    task_retrieve_value(hw_vm);
    task_retrieve_value(hw_rss);
    task_retrieve_value(rbytes);
    task_retrieve_value(wbytes);
    task_retrieve_value(cancel_wbytes);
    task_retrieve_value(utsns);
    task_retrieve_value(ipcns);
    task_retrieve_value(mntns);
    task_retrieve_value(pidns);
    task_retrieve_value(netns);
    task_retrieve_value(cgroupns);
  } else if(edge_type(edge) == RL_VERSION && prov_is_inode(node_type(node))) {
    mode_retrieve_value(mode);
  }
  return 0;
}

QUERY_DESCRIPTION("Compute feature vectors for unicorn!");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier and Michael Han");
QUERY_VERSION("0.1");
QUERY_NAME("Unicorn");

// service specific build settings
#ifdef SERVICE_QUERY
QUERY_CHANNEL("unicorn");
QUERY_OUTPUT("/tmp/unicorn.log");
#endif

register_query(init, in_edge, out_edge);
