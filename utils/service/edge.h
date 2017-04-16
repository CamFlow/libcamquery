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
#ifndef ___LIB_QUERY_EDGE_H
#define ___LIB_QUERY_EDGE_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "utils.h"

static pthread_mutex_t c_lock_edge = PTHREAD_MUTEX_INITIALIZER;
static struct hashable_edge *edge_hash_table = NULL;

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

static inline int insert_edge(prov_entry_t *elt){
  struct hashable_edge *edge = (struct hashable_edge*) malloc(sizeof(struct hashable_edge));
  memset(edge, 0, sizeof(struct hashable_edge));
  edge->msg = elt;
  clock_gettime(CLOCK_REALTIME, &(edge->t_exist));
  memcpy(&edge->key, &(elt->relation_info.identifier.relation_id), sizeof(struct relation_identifier));
  pthread_mutex_lock(&c_lock_edge);
  HASH_ADD(hh, edge_hash_table, key, sizeof(struct relation_identifier), edge);
  pthread_mutex_unlock(&c_lock_edge);
}

static inline void delete_edge_nolock(struct hashable_edge *edge){
  HASH_DEL(edge_hash_table, edge);
  free(edge->msg);
  free(edge);
}

static inline uint32_t edge_count_nolock(void){
  return HASH_COUNT(edge_hash_table);
}
#endif
