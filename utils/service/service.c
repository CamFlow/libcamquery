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
#define WIN_SIZE 1
#define WAIT_TIME 3
#define STALL_TIME 2*WAIT_TIME


static struct timespec t_cur;

// per thread init
static void init( void ){
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

static inline void get_nodes(struct hashable_edge *edge,
                              struct hashable_node **from_node,
                              struct hashable_node **to_node){
  *from_node = get_node(&edge->msg->relation_info.snd.node_id);
  *to_node = get_node(&edge->msg->relation_info.rcv.node_id);
}

static inline bool handle_missing_nodes(struct hashable_edge *edge, struct hashable_node *from_node, struct hashable_node *to_node){
  if (time_elapsed(edge->t_exist, t_cur).tv_sec < STALL_TIME)
    return false;
#ifdef DEBUG
  print("****THIS EDGE HAS BEEN STALLED TOO LONG****");
  print("%s %s", "Stuck Edge Type: ", relation_str(prov_type(edge->msg)));
  if (from_node == NULL) {
    print("%s %s", "Stuck Edge From Node Type: ", node_str(edge->msg->relation_info.snd.node_id.type));
    print("%s %d %d %d %d", "Stuck node id: ", edge->msg->relation_info.snd.node_id.type,
    edge->msg->relation_info.snd.node_id.id, edge->msg->relation_info.snd.node_id.boot_id,
    edge->msg->relation_info.snd.node_id.version);
  }
  if (to_node == NULL) {
    print("%s %s", "Stuck Edge To Node Type: ", node_str(edge->msg->relation_info.rcv.node_id.type));
    print("%s %d %d %d %d", "Stuck node id: ", edge->msg->relation_info.rcv.node_id.type,
    edge->msg->relation_info.rcv.node_id.id, edge->msg->relation_info.rcv.node_id.boot_id,
    edge->msg->relation_info.rcv.node_id.version);
  }
#endif
  //*****************
  delete_edge_nolock(edge);
  return true;
}

static inline void process() {
  struct hashable_node *from_node, *to_node;
  struct hashable_edge *edge, *tmp;

  clock_gettime(CLOCK_REALTIME, &t_cur);
  HASH_ITER(hh, edge_hash_table, edge, tmp) {
    // too recent we come back later.
    if (time_elapsed(edge->t_exist, t_cur).tv_sec < WAIT_TIME)
      return;
    get_nodes(edge, &from_node, &to_node);
    if (!from_node || !to_node) { // we cannot find one of the node
      if(handle_missing_nodes(edge, from_node, to_node))
        continue;
      else
        return;
    }
#ifdef DEBUG
    print("======Processing an edge======");
#endif
    /*
    * GARBAGE COLLECTION
    */
    if ( clean_incoming(from_node->msg, edge->msg) ) {
      delete_node(from_node);
    } else if ( clean_bothend(edge->msg) ) {
      delete_node(from_node);
      delete_node(to_node);
    }
    delete_edge_nolock(edge);
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
  .init=&init,
  .received_prov=&received_prov,
  .received_long_prov=&received_long_prov,
  .log_error=&log_error
};

int main(void){
  int rc;
  unsigned int before_node_table, before_edge_table, after_node_table, after_edge_table;

  _init_logs();
  print("Runtime query service pid: %ld\n", getpid());
  rc = provenance_register(&ops);
  if(rc<0){
    print("Failed registering audit operation (%d).\n", rc);
    exit(rc);
  }
  print("=====STARTING====");
  while(1){
    pthread_mutex_lock(&c_lock_edge);
    HASH_SORT(edge_hash_table, edge_compare);
#ifdef DEBUG
    before_node_table = node_count();
    before_edge_table = edge_count_nolock();
#endif
    process();
#ifdef DEBUG
    after_node_table = node_count();
    after_edge_table = edge_count_nolock();
#endif
    pthread_mutex_unlock(&c_lock_edge);
#ifdef DEBUG
    print("%s %u", "Hash Table (Nodes) Size Before: ", before_node_table);
    print("%s %u", "Hash Table (Edges) Size Before: ", before_edge_table);
    print("%s %u", "Hash Table (Nodes) Size After: ", after_node_table);
    print("%s %u", "Hash Table (Edges) Size After: ", after_edge_table);
#endif
    sleep(1);
  }
  provenance_stop();
  return 0;
}
