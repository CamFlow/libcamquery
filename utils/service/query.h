/*
 *
 * Author: Thomas Pasquier <tfjmp@g.harvard.edu>
 * Author: Xueyuan Michael Han <hanx@g.harvard.edu>
 *
 * Copyright (C) 2017 Harvard University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 */
#ifndef ___LIB_QUERY_QUERY_H
#define ___LIB_QUERY_QUERY_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>

#include "provenancelib.h"
#include "provenanceutils.h"
#include "provenancePovJSON.h"
#include "uthash.h"
#include "node.h"
#include "edge.h"
#include "utils.h"

#define gettid() syscall(SYS_gettid)

#define CAMFLOW_RAISE_WARNING -1

#define WIN_SIZE 1
#define WAIT_TIME 1
#define STALL_TIME 1*WAIT_TIME

int (*out_edge_ptr)(prov_entry_t*, prov_entry_t*)=NULL;
int (*in_edge_ptr)(prov_entry_t*, prov_entry_t*)=NULL;

static struct hashable_query *query_hash_table = NULL;

static inline void runquery(prov_entry_t *from, prov_entry_t *edge, prov_entry_t *to){
  struct hashable_query *query, *tmp;
  int rc=0;
  rc = out_edge_ptr(from, edge);
  if(rc<0){
    print("out: %d", rc);
  }
  rc = in_edge_ptr(edge, to);
  if(rc<0){
    print("in: %d", rc);
  }
}

static struct timespec t_cur;

// per thread init
static void ___init( void ){
  pid_t tid = gettid();
  print("audit writer thread, tid:%ld\n", tid);
}

static inline bool clean_incoming(prov_entry_t *from,
                                  prov_entry_t *edge){
  if (prov_type(edge) == RL_VERSION || prov_type(edge) == RL_VERSION_PROCESS)
    return true;
  if (prov_type(edge) == RL_NAMED_PROCESS || prov_type(edge) == RL_NAMED)
    return true;
  if (prov_type(from) == ENT_PACKET
      || prov_type(from) == ENT_XATTR
      || prov_type(from) == ENT_IATTR)
    return true;
  return false;
}

static inline bool clean_bothend(prov_entry_t *edge){
  if (prov_type(edge) == RL_CLOSED || prov_type(edge) == RL_TERMINATE_PROCESS)
    return true;
  return false;
}

static inline void get_nodes(struct edge *edge,
                              struct hashable_node **from_node,
                              struct hashable_node **to_node){
  *from_node = get_node(&edge->msg->relation_info.snd.node_id);
  *to_node = get_node(&edge->msg->relation_info.rcv.node_id);
}

static inline bool handle_missing_nodes(struct edge *edge, struct hashable_node *from_node, struct hashable_node *to_node){
  if (time_elapsed(edge->t_exist, t_cur).tv_sec < STALL_TIME)
    return false;
  free(edge->msg);
  free(edge);
  return true;
}

static inline void process() {
  struct hashable_node *from_node, *to_node;
  struct edge* edge;
  clock_gettime(CLOCK_REALTIME, &t_cur);
  if(bundle_head()==NULL)
    return;
  if (time_elapsed(bundle_head()->t_exist, t_cur).tv_sec < WAIT_TIME)
    return;
  edge = edge_pop(&bundle.list[0]);
  while(edge!=NULL){
    get_nodes(edge, &from_node, &to_node);
    if (!from_node || !to_node) { // we cannot find one of the node
      if(handle_missing_nodes(edge, from_node, to_node)){
        edge = edge_pop(&bundle.list[0]);
        continue;
      } else{
        // we put it back in the list
        insert_in_bundle(edge->msg);
        free(edge);
        return;
      }
    }
    // we run the query
    runquery(from_node->msg, edge->msg, to_node->msg);
    // we garbage collect the nodes
    if ( clean_incoming(from_node->msg, edge->msg) ) {
      delete_node(from_node);
    } else if ( clean_bothend(edge->msg) ) {
      delete_node(from_node);
      delete_node(to_node);
    }
    free(edge->msg);
    free(edge);
    if (time_elapsed(bundle_head()->t_exist, t_cur).tv_sec < WAIT_TIME)
      return;
    edge = edge_pop(&bundle.list[0]);
  }
}

static inline void record(prov_entry_t* elt){
  if(prov_is_relation(elt))
    insert_edge(elt);
  else
    insert_node(elt);
}

static void received_prov(union prov_elt* msg){
  prov_entry_t* elt = (prov_entry_t*)malloc(sizeof(union prov_elt));
  memcpy(elt, msg, sizeof(union prov_elt));
  record(elt);
}

static void received_long_prov(union long_prov_elt* msg){
  prov_entry_t* elt = (prov_entry_t*)malloc(sizeof(union long_prov_elt));
  memcpy(elt, msg, sizeof(union long_prov_elt));
  record(elt);
}

static void log_error(char* err_msg){
  print(err_msg);
}

struct provenance_ops ops = {
  .init=&___init,
  .received_prov=&received_prov,
  .received_long_prov=&received_long_prov,
  .log_error=&log_error
};

#define register_query(init_fcn, in_fcn, out_fcn)\
int main(void){\
  int rc;\
  _init_logs();\
  in_edge_ptr=in_fcn;\
  out_edge_ptr=out_fcn;\
  init_fcn();\
  print("Runtime query service pid: %ld\n", getpid());\
  rc = provenance_register(&ops);\
  if(rc<0){\
    print("Failed registering audit operation (%d).\n", rc);\
    exit(rc);\
  }\
  while(1){\
    pthread_mutex_lock(&c_lock_edge);\
    print("Bundle number %u", bundle_count_nolock());\
    print("Bundle size %u", edge_count_nolock());\
    print("Before merge\n");\
    merge_bundle();\
    print("After merge\n");\
    process();\
    print("After process\n");\
    pthread_mutex_unlock(&c_lock_edge);\
    sleep(1);\
  }\
  provenance_stop();\
  return 0;\
}

typedef uint64_t label_t;

#define assign_label(name, str) name = generate_label(str)

static inline bool has_label(prov_entry_t* elmt, label_t label){
  return prov_bloom_in(prov_taint(elmt), label);
}

static inline void add_label(prov_entry_t* elmt, label_t label){
  prov_bloom_add(prov_taint(elmt), label);
}

 #endif
