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
    case ENT_INODE_SOCKET:
      write_socket(node);
      break;
    case ENT_INODE_FIFO:
      write_fifo(node);
      break;
    case ENT_INODE_LINK:
      write_link(node);
      break;
    case ENT_INODE_MMAP:
      write_mmap(node);
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

#define propagate_value(type, cont, param) ep->param = node->cont.param; memcpy(ep->p_ ## param, np->p_ ## param, REC_SIZE*sizeof(type))

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  struct vec *ep;
  struct vec *np;

  if (edge->msg_info.var_ptr == NULL)
    edge->msg_info.var_ptr = zalloc(sizeof(struct vec));
  ep = edge->msg_info.var_ptr;

  if (node->msg_info.var_ptr == NULL)
    node->msg_info.var_ptr = zalloc(sizeof(struct vec));
  np = node->msg_info.var_ptr;

  propagate_value(uint64_t, task_info, utime);
  propagate_value(uint64_t, task_info, stime);
  propagate_value(uint64_t, task_info, vm);
  propagate_value(uint64_t, task_info, rss);
  propagate_value(uint64_t, task_info, hw_vm);
  propagate_value(uint64_t, task_info, hw_rss);
  propagate_value(uint64_t, task_info, rbytes);
  propagate_value(uint64_t, task_info, wbytes);
  propagate_value(uint64_t, task_info, cancel_wbytes);

  propagate_value(uint32_t, task_info, utsns);
  propagate_value(uint32_t, task_info, ipcns);
  propagate_value(uint32_t, task_info, mntns);
  propagate_value(uint32_t, task_info, pidns);
  propagate_value(uint32_t, task_info, netns);
  propagate_value(uint32_t, task_info, cgroupns);

  propagate_value(uint16_t, inode_info, mode);

  ep->type = node_type(node);
  memcpy(ep->p_type, np->p_type, REC_SIZE*sizeof(uint64_t));
  ep->in_type = edge_type(edge);
  memcpy(ep->p_in_type, np->p_in_type, REC_SIZE*sizeof(uint64_t));
  ep->uid = node->msg_info.uid;
  memcpy(ep->p_uid, np->p_uid, REC_SIZE*sizeof(uint64_t));
  ep->gid = node->msg_info.uid;
  memcpy(ep->p_gid, np->p_gid, REC_SIZE*sizeof(uint64_t));
  print_node(node);
  return 0;
}

#define retrieve_ancestry(param) np->p_ ## param[np->in] = ep->param;\
                                for(i=1; i<MAX_DEPTH; i++){\
                                  pos = CALC(i) + np->in*pwrtwo(i);\
                                  nb = pwrtwo(i);\
                                  pos2 = CALC(i-1);\
                                  for(j=0;j<nb;j++){\
                                    np->p_ ## param[pos+j]=ep->p_ ## param[pos2+j];\
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
    retrieve_ancestry(in_type);
    retrieve_ancestry(type);
    retrieve_ancestry(uid);
    retrieve_ancestry(gid);
    retrieve_ancestry(utime);
    retrieve_ancestry(stime);
    retrieve_ancestry(vm);
    retrieve_ancestry(rss);
    retrieve_ancestry(hw_vm);
    retrieve_ancestry(hw_rss);
    retrieve_ancestry(rbytes);
    retrieve_ancestry(wbytes);
    retrieve_ancestry(cancel_wbytes);
    retrieve_ancestry(utsns);
    retrieve_ancestry(ipcns);
    retrieve_ancestry(mntns);
    retrieve_ancestry(pidns);
    retrieve_ancestry(netns);
    retrieve_ancestry(cgroupns);
    retrieve_ancestry(mode);
  }
  np->in++;
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
