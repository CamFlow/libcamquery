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
  print("unicornd ready!")
}

static void print_node(prov_entry_t* node){
  switch(node_type(node)){
    case ACT_TASK:
      //write_task(node);
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
    ep->utime = node->task_info.utime;
    ep->stime = node->task_info.stime;
    ep->vm = node->task_info.vm;
    ep->rss = node->task_info.rss;
    ep->hw_vm = node->task_info.hw_vm;
    ep->hw_rss = node->task_info.hw_rss;
    ep->rbytes = node->task_info.rbytes;
    ep->wbytes = node->task_info.wbytes;
    ep->cancel_wbytes = node->task_info.cancel_wbytes;
  }
  ep->p_type[0] = node_type(node);
  print_node(node);
  return 0;
}

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  struct vec *ep;
  struct vec *np;

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
    np->utime = node->task_info.utime - ep->utime;
    np->stime = node->task_info.stime - ep->stime;
    np->vm = node->task_info.vm - ep->vm;
    np->rss = node->task_info.rss - ep->rss;
    np->hw_vm = node->task_info.hw_vm - ep->hw_vm;
    np->hw_rss = node->task_info.hw_rss - ep->hw_rss;
    np->rbytes = node->task_info.rbytes - ep->rbytes;
    np->wbytes = node->task_info.wbytes - ep->wbytes;
    np->cancel_wbytes = node->task_info.cancel_wbytes - ep->cancel_wbytes;
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
