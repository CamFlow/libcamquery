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
#ifndef ___LIB_QUERY_NODE_H
#define ___LIB_QUERY_NODE_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "utils.h"

struct hashable_node {
  prov_entry_t *msg;
  struct timespec t_exist;
  struct node_identifier key;
  UT_hash_handle hh;
};

static pthread_mutex_t c_lock_node = PTHREAD_MUTEX_INITIALIZER;
static struct hashable_node *node_hash_table = NULL;

static inline int insert_node(prov_entry_t *elt){
  struct hashable_node *node;
  node = (struct hashable_node*) malloc(sizeof(struct hashable_node));
  memset(node, 0, sizeof(struct hashable_node));
  node->msg = elt;
  clock_gettime(CLOCK_REALTIME, &(node->t_exist));
  memcpy(&node->key, &(elt->node_info.identifier.node_id), sizeof(struct node_identifier));
  pthread_mutex_lock(&c_lock_node);
  HASH_ADD(hh, node_hash_table, key, sizeof(struct node_identifier), node);
  pthread_mutex_unlock(&c_lock_node);
}

static inline void delete_node(struct hashable_node *node){
  pthread_mutex_lock(&c_lock_node);
  HASH_DEL(node_hash_table, node);
  pthread_mutex_unlock(&c_lock_node);
#ifdef DEBUG
  print("Deleting the node: %d %d %d %d", node->msg->node_info.identifier.node_id.type,
  node->msg->node_info.identifier.node_id.id, node->msg->node_info.identifier.node_id.boot_id,
  node->msg->node_info.identifier.node_id.version);
#endif
  free(node->msg);
  free(node);
}

static inline struct hashable_node* get_node(struct node_identifier *id){
  struct hashable_node* node;
  pthread_mutex_lock(&c_lock_node);
  HASH_FIND(hh, node_hash_table, id, sizeof(struct node_identifier), node);
  pthread_mutex_unlock(&c_lock_node);
  return node;
}

static inline uint32_t node_count(void){
  uint32_t c;
  pthread_mutex_lock(&c_lock_node);
  c = HASH_COUNT(node_hash_table);
  pthread_mutex_unlock(&c_lock_node);
  return c;
}

static inline void node_print_content(void){
  struct hashable_node *node, *tmp;
  pthread_mutex_lock(&c_lock_node);
  HASH_ITER(hh, node_hash_table, node, tmp) {
    print("Uncleared Node Type: %s", node_str(prov_type(node->msg)));
  }
  pthread_mutex_unlock(&c_lock_node);
}
#endif
