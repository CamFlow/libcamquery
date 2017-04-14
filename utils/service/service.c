
/*
 *
 * Author: Thomas Pasquier <tfjmp@g.harvard.edu>
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

#define	LOG_FILE "/tmp/audit.log"
#define gettid() syscall(SYS_gettid)
#define WIN_SIZE 1
#define WAIT_TIME 3
#define STALL_TIME 1*WAIT_TIME

static pthread_mutex_t l_log =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t c_lock_edge = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t c_lock_node = PTHREAD_MUTEX_INITIALIZER;
static struct timespec t_cur;

FILE *fp=NULL;

void _init_logs( void ){
 int n;
 fp = fopen(LOG_FILE, "a+");
 if(!fp){
   printf("Cannot open file\n");
   exit(-1);
 }
 n = fprintf(fp, "Starting audit service...\n");
 printf("%d\n", n);

 provenance_opaque_file(LOG_FILE, true);
}

#define print(fmt, ...)\
    pthread_mutex_lock(&l_log);\
    fprintf(fp, fmt, ##__VA_ARGS__);\
    fprintf(fp, "\n");\
    fflush(fp);\
    pthread_mutex_unlock(&l_log);

// per thread init
void init( void ){
 pid_t tid = gettid();
 print("audit writer thread, tid:%ld\n", tid);
}

struct timespec time_elapsed(struct timespec start, struct timespec end) {
  struct timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp;
}

struct hashable_node {
  prov_entry_t *msg;
  struct timespec t_exist;
  struct node_identifier key;
  UT_hash_handle hh;
};

struct hashable_edge {
  prov_entry_t *msg;
  struct timespec t_exist;
  struct relation_identifier key;
  UT_hash_handle hh;
};

int edge_compare(struct hashable_edge* he1, struct hashable_edge* he2) {
  uint32_t he1_id = he1->msg->relation_info.identifier.relation_id.id;
  uint32_t he2_id = he2->msg->relation_info.identifier.relation_id.id;
  uint32_t he1_boot = he1->msg->relation_info.identifier.relation_id.boot_id;
  uint32_t he2_boot = he2->msg->relation_info.identifier.relation_id.boot_id;
  if (he1_boot < he2_boot) return -1;
  else if (he1_boot > he2_boot) return 1;
  else {//if both edges are in the same boot, compare their ids
    if (he1_id > he2_id) return 1;
    else if (he1_id == he2_id) return 0;
    else return -1;
  }
}

static struct hashable_node *node_hash_table = NULL;
static struct hashable_edge *edge_hash_table = NULL;

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

static inline bool clean_packet(struct hashable_node *to){
  if (prov_type(to->msg) == ENT_PACKET && time_elapsed(to->t_exist, t_cur).tv_sec > STALL_TIME)
    return true;
  else
    return false;
}

static inline void delete_node(struct hashable_node *node){
  HASH_DEL(node_hash_table, node);
  #ifdef DEBUG
  print("%s %d %d %d %d", "Deleting the node: ", node->msg->node_info.identifier.node_id.type,
  node->msg->node_info.identifier.node_id.id, node->msg->node_info.identifier.node_id.boot_id,
  node->msg->node_info.identifier.node_id.version);
  #endif
  free(node->msg);
  free(node);
}

static inline void delete_edge(struct hashable_edge *edge){
  HASH_DEL(edge_hash_table, edge);
  free(edge->msg);
  free(edge);
}

static inline void get_nodes(struct hashable_edge *edge,
                              struct hashable_node **from_node,
                              struct hashable_node **to_node){
  struct node_identifier from, to;
  memcpy(&from, &edge->msg->relation_info.snd.node_id, sizeof(struct node_identifier));
  memcpy(&to, &edge->msg->relation_info.rcv.node_id, sizeof(struct node_identifier));
  HASH_FIND(hh, node_hash_table, &from, sizeof(struct node_identifier), *from_node);
  HASH_FIND(hh, node_hash_table, &to, sizeof(struct node_identifier), *to_node);
}

static inline bool handle_missing_nodes(struct hashable_edge *edge, struct hashable_node *from_node, struct hashable_node *to_node){
  if (time_elapsed(edge->t_exist, t_cur).tv_sec < STALL_TIME)
    return false;
  #ifdef DEBUG
  print("****THIS EDGE HAS BEEN STALLED TOO LONG****");
  /*
  * For debug only
  */
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
  delete_edge(edge);
  return true;
}

void process() {
  struct hashable_node *from_node, *to_node;
  struct hashable_edge *edge, *tmp;

  clock_gettime(CLOCK_REALTIME, &t_cur);
  HASH_ITER(hh, edge_hash_table, edge, tmp) {
    get_nodes(edge, &from_node, &to_node);
    if (from_node && to_node) {
      //print("=====Found Both Nodes======");
      if (time_elapsed(edge->t_exist, t_cur).tv_sec >= WAIT_TIME) {
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
        } else if ( clean_packet(to_node) )
          delete_node(to_node);
        delete_edge(edge);
      }else
        break;
    }else if ( !handle_missing_nodes(edge, from_node, to_node) )
        break;
  }
}

bool filter(prov_entry_t* msg){
  prov_entry_t* elt = malloc(sizeof(prov_entry_t));
  struct hashable_edge *edge;
  struct hashable_node *node;

  memcpy(elt, msg, sizeof(prov_entry_t));
  if(prov_is_relation(elt)) {
    edge = (struct hashable_edge*) malloc(sizeof(struct hashable_edge));
    memset(edge, 0, sizeof(struct hashable_edge));
    edge->msg = elt;
    clock_gettime(CLOCK_REALTIME, &(edge->t_exist));
    memcpy(&edge->key, &(elt->relation_info.identifier.relation_id), sizeof(struct relation_identifier));
    pthread_mutex_lock(&c_lock_edge);
    HASH_ADD(hh, edge_hash_table, key, sizeof(struct relation_identifier), edge);
    pthread_mutex_unlock(&c_lock_edge);
  } else{
    node = (struct hashable_node*) malloc(sizeof(struct hashable_node));
    memset(node, 0, sizeof(struct hashable_node));
    node->msg = elt;
    clock_gettime(CLOCK_REALTIME, &(node->t_exist));
    memcpy(&node->key, &(elt->node_info.identifier.node_id), sizeof(struct node_identifier));
    pthread_mutex_lock(&c_lock_node);
    HASH_ADD(hh, node_hash_table, key, sizeof(struct node_identifier), node);
    pthread_mutex_unlock(&c_lock_node);
  }
  return false;
}

void log_error(char* err_msg){
  print(err_msg);
}

struct provenance_ops ops = {
 .init=&init,
 .filter=&filter,
 .log_error=&log_error
};

int main(void){
 int rc;
 char json[4096];
 unsigned int before_node_table, before_edge_table, after_node_table, after_edge_table;
 /*
 * For debug only
 */
 struct hashable_node *node, *tmp;
 //*******************
	_init_logs();
 fprintf(fp, "Runtime query service pid: %ld\n", getpid());
 rc = provenance_register(&ops);
 if(rc<0){
   fprintf(fp, "Failed registering audit operation (%d).\n", rc);
   exit(rc);
 }
 fprintf(fp, "=====STARTING====");
 fprintf(fp, "\n");
 fflush(fp);

 while(1){
   pthread_mutex_lock(&c_lock_edge);
   pthread_mutex_lock(&c_lock_node);
   HASH_SORT(edge_hash_table, edge_compare);
   #ifdef DEBUG
   before_node_table = HASH_COUNT(node_hash_table);
   before_edge_table = HASH_COUNT(edge_hash_table);
   #endif
   process();
   #ifdef DEBUG
   after_node_table = HASH_COUNT(node_hash_table);
   after_edge_table = HASH_COUNT(edge_hash_table);
   #endif
   pthread_mutex_unlock(&c_lock_node);
   pthread_mutex_unlock(&c_lock_edge);
   #ifdef DEBUG
   print("%s %u", "Hash Table (Nodes) Size Before: ", before_node_table);
   print("%s %u", "Hash Table (Edges) Size Before: ", before_edge_table);
   print("%s %u", "Hash Table (Nodes) Size After: ", after_node_table);
   print("%s %u", "Hash Table (Edges) Size After: ", after_edge_table);
   #endif
   sleep(1);
   /*
   * For debug only
   * Find out the type of the remaining nodes still in the node hash table
   */
   #ifdef DEBUG
   if (HASH_COUNT(edge_hash_table) == 0) {
      HASH_ITER(hh, node_hash_table, node, tmp) {
        print("%s, %s", "Uncleared Node Type: ", node_str(prov_type(node->msg)));
      }
   }
   #endif
   //*********************************
 }
 provenance_stop();
 return 0;
}
