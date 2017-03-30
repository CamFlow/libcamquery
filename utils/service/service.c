
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
#include "libut.h"

#define	LOG_FILE "/tmp/audit.log"
#define gettid() syscall(SYS_gettid)
#define WIN_SIZE 100
#define WAIT_TIME 2

static pthread_mutex_t l_log =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t c_lock = PTHREAD_MUTEX_INITIALIZER;

FILE *fp=NULL;

int counter = 0;

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

static inline void print(char* str){
    pthread_mutex_lock(&l_log);
    fprintf(fp, str);
    fprintf(fp, "\n");
    fflush(fp);
    pthread_mutex_unlock(&l_log);
}

// per thread init
void init( void ){
 pid_t tid = gettid();
 pthread_mutex_lock(&l_log);
 fprintf(fp, "audit writer thread, tid:%ld\n", tid);
 pthread_mutex_unlock(&l_log);
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
  prov_entry_t msg;
  struct node_identifier key;
  UT_hash_handle hh;
};

struct hashable_edge {
  prov_entry_t msg;
  struct timespec t_exist;
  struct hashable_edge *next;
};

int edge_compare(struct hashable_edge* he1, struct hashable_edge* he2) {
  uint32_t he1_id = he1->msg.relation_info.identifier.relation_id.id;
  uint32_t he2_id = he2->msg.relation_info.identifier.relation_id.id;
  if (he1_id > he2_id) return 1;
  else if (he1_id == he2_id) return 0;
  else return -1;
}

static struct hashable_node *node_hash_table = NULL;
static struct hashable_edge *edge_hash_head = NULL;

void process(struct hashable_node* nodes, struct hashable_edge* head_edge) {
  struct timespec t_cur;
  clock_gettime(CLOCK_REALTIME, &t_cur);
  struct hashable_edge *elt, *tmp = NULL;
  LL_FOREACH_SAFE(head_edge, elt, tmp) {
    //Find nodes of the edge
    struct node_identifier from = elt->msg.relation_info.snd.node_id;
    struct node_identifier to = elt->msg.relation_info.rcv.node_id;
    struct hashable_node *from_node, *to_node = NULL;
    HASH_FIND(hh, nodes, &from, sizeof(struct node_identifier), from_node);
    HASH_FIND(hh, nodes, &to, sizeof(struct node_identifier), to_node);
    if (from_node == NULL) {
      print ("From node not found");
    }
    if (to_node == NULL) {
      print ("To node not found");
    }
    //do something with the nodes if both nodes are found
    if (from_node && to_node) {
      print("=====Found Both Nodes======");
      //and if the edge has been in the window for a predetermined time
      if (time_elapsed(elt->t_exist, t_cur).tv_sec >= WAIT_TIME) {
        //TODO: write code here
        print("======Processing an edge======");
        //garbage collect the edge
        LL_DELETE(head_edge, elt);
        counter--;
        //garbage collect from_node if same node but new version is found
        if (W3C_TYPE(prov_type(&elt->msg)) == RL_VERSION || W3C_TYPE(prov_type(&elt->msg)) == RL_VERSION_PROCESS) {
          HASH_DEL(nodes, from_node);
          free(from_node);
        }
        free(elt);
      } else {
        print("Going to break here...");
        break;
      }
    } else {
      print("Break again here...");
      break;
    }
  }
}

bool filter(prov_entry_t* msg){
  prov_entry_t* elt = malloc(sizeof(prov_entry_t));
  memcpy(elt, msg, sizeof(prov_entry_t));

  if(prov_is_relation(msg)) {
    struct hashable_edge *edge = NULL;
    edge = (struct hashable_edge*) malloc(sizeof(struct hashable_edge));
    memset(edge, 0, sizeof(struct hashable_edge));
    edge->msg = *msg;
    clock_gettime(CLOCK_REALTIME, &(edge->t_exist));
    pthread_mutex_lock(&c_lock);
    LL_APPEND(edge_hash_head, edge);
    counter++;
    pthread_mutex_unlock(&c_lock);
  } else{
    struct hashable_node *node = NULL;
    node = (struct hashable_node*) malloc(sizeof(struct hashable_node));
    memset(node, 0, sizeof(struct hashable_node));
    node->msg = *msg;
    node->key = msg->node_info.identifier.node_id;
    pthread_mutex_lock(&c_lock);
    HASH_ADD(hh, node_hash_table, key, sizeof(struct node_identifier), node);
    pthread_mutex_unlock(&c_lock);
  }

  pthread_mutex_lock(&c_lock);
  if (counter >= WIN_SIZE) {
    LL_SORT(edge_hash_head, edge_compare);
    // struct hashable_edge *elt;
    // int count_edges;
    // pthread_mutex_lock(&l_log);
    // fprintf(fp, "%s %u", "Hash Table (Nodes) Size Before: ", HASH_COUNT(node_hash_table));
    // fprintf(fp, "\n");
    // fflush(fp);
    // pthread_mutex_unlock(&l_log);
    // LL_COUNT(edge_hash_head, elt, count_edges);
    // pthread_mutex_lock(&l_log);
    // fprintf(fp, "%s %d", "Hash List (Edges) Size Before: ", count_edges);
    // fprintf(fp, "\n");
    // fflush(fp);
    // pthread_mutex_unlock(&l_log);
    process(node_hash_table, edge_hash_head);
    // pthread_mutex_lock(&l_log);
    // fprintf(fp, "%s %u", "Hash Table (Nodes) Size After: ", HASH_COUNT(node_hash_table));
    // fprintf(fp, "\n");
    // fflush(fp);
    // pthread_mutex_unlock(&l_log);
    //LL_COUNT(edge_hash_head, elt, count_edges);
    //pthread_mutex_lock(&l_log);
    //fprintf(fp, "%s %d", "Hash List (Edges) Size After: ", count_edges);
    //fprintf(fp, "\n");
    //fflush(fp);
    //pthread_mutex_unlock(&l_log);
  }
  pthread_mutex_unlock(&c_lock);

  print("Received an entry!");
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
 fprintf(fp, machine_description_json(json));
 fprintf(fp, "\n");
 fflush(fp);

 while(1){
   sleep(1);
 }
 provenance_stop();
 return 0;
}
