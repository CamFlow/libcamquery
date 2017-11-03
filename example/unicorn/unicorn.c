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
  }
  ep->p_type[0] = node_type(node);
  print_node(node);
  return 0;
}

#define task_retrieve_value(param) memcpy(np->param, ep->param, MAX_DEPTH*sizeof(uint64_t))

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  struct vec *ep;
  struct vec *np;
  int i;

  if (edge->msg_info.var_ptr == NULL)
    edge->msg_info.var_ptr = zalloc(sizeof(struct vec));
  ep = edge->msg_info.var_ptr;

  if (node->msg_info.var_ptr == NULL)
    node->msg_info.var_ptr = zalloc(sizeof(struct vec));
  np = node->msg_info.var_ptr;

  /* couting in edges */
  if (np->in < MAX_PARENT) {
    np->in_type[np->in] = edge_type(edge);
    np->p_type[np->in] = ep->p_type[0];
    np->offset[np->in] = edge->relation_info.offset;
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
