
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
#define WAIT_TIME 2

static pthread_mutex_t l_log =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t c_lock_edge = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t c_lock_node = PTHREAD_MUTEX_INITIALIZER;

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
  if (he1_id > he2_id) return 1;
  else if (he1_id == he2_id) return 0;
  else return -1;
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
      || prov_type(from) == ENT_XATTR)
    return true;
  return false;
}

static inline bool clean_bothend(prov_entry_t *edge){
  if (prov_type(edge) == RL_CLOSED || prov_type(edge) == RL_TERMINATE_PROCESS)
    return true;
  return false;
}

static inline void delete_node(struct hashable_node *node){
  HASH_DEL(node_hash_table, node);
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

static inline bool handle_missing_nodes(struct hashable_edge *edge){
  struct timespec t_cur;
  clock_gettime(CLOCK_REALTIME, &t_cur);
  if (time_elapsed(edge->t_exist, t_cur).tv_sec < 5*WAIT_TIME)
    return false;
  print("****THIS EDGE HAS BEEN STALLED TOO LONG****");
  delete_edge(edge);
  return true;
}

void process() {
  struct timespec t_cur;
  struct hashable_node *from_node, *to_node;
  struct hashable_edge *edge, *tmp;

  clock_gettime(CLOCK_REALTIME, &t_cur);
  HASH_ITER(hh, edge_hash_table, edge, tmp) {
    get_nodes(edge, &from_node, &to_node);
    if (from_node && to_node) {
      print("=====Found Both Nodes======");
      if (time_elapsed(edge->t_exist, t_cur).tv_sec >= WAIT_TIME) {
        print("======Processing an edge======");

        /*
        * GARBAGE COLLECTION
        */
        if ( clean_incoming(from_node->msg, edge->msg) ) {
          delete_node(from_node);
        } else if ( clean_bothend(edge->msg) ) {
          delete_node(from_node);
          delete_node(to_node);
        }
        delete_edge(edge);
      }else
        break;
    }else if ( !handle_missing_nodes(edge) )
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
   print("%s %u", "Hash Table (Nodes) Size Before: ", HASH_COUNT(node_hash_table));
   print("%s %u", "Hash Table (Edges) Size Before: ", HASH_COUNT(edge_hash_table));
   process();
   print("%s %u", "Hash Table (Nodes) Size After: ", HASH_COUNT(node_hash_table));
   print("%s %u", "Hash Table (Edges) Size After: ", HASH_COUNT(edge_hash_table));
   pthread_mutex_unlock(&c_lock_node);
   pthread_mutex_unlock(&c_lock_edge);
   sleep(1);
 }
 provenance_stop();
 return 0;
}
