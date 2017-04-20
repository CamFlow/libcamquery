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

#define MAX_BUNDLE 100

static pthread_mutex_t c_lock_edge = PTHREAD_MUTEX_INITIALIZER;

struct bundle {
  struct edge *list[MAX_BUNDLE];
  prov_entry_t *last[MAX_BUNDLE];
} bundle;

struct edge
{
    prov_entry_t *msg;
    struct timespec t_exist;
    struct node *next;
};

static inline bool edge_greater_than(prov_entry_t *edge1, prov_entry_t *edge2) {
  uint32_t he1_id = edge1->relation_info.identifier.relation_id.id;
  uint32_t he2_id = edge2->relation_info.identifier.relation_id.id;
  uint32_t he1_boot = edge1->relation_info.identifier.relation_id.boot_id;
  uint32_t he2_boot = edge2->relation_info.identifier.relation_id.boot_id;
  if (he1_boot > he2_boot)
    return true;
  else if (he1_id > he2_id)
    return true;
  return false;
}

void append(struct edge **list, prov_entry_t* msg)
{
    struct edge *temp,*right;
    struct edge *head;
    head = *list;
    temp= (struct edge *)malloc(sizeof(struct edge));
    temp->msg = msg;
    if (head == NULL)
    {
      *list=temp;
      (*list)->next=NULL;
      return;
    }
    right=(struct node *)head;
    while(right->next != NULL)
      right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}



void insert(struct edge **list, struct edge *edge){
    struct edge *temp;
    temp=*list;
    if (temp==NULL) {
      append(list, edge);
    } else {
      while(temp!=NULL)
      {
          if(temp->next==NULL) {
            temp->next = edge;
          } else {
            if( edge_greater_than(temp->next, edge->msg) ) {
              edge->next = temp->next;
              temp->next = edge;
              return;
            }
          }
          temp = temp->next;
      }
    }
}

struct edge* pop(struct edge **list){
  struct edge *tmp = (*list);
  (*list) = tmp->next;
  return tmp;
}

void merge(struct edge **main, sturct edge *small){
  struct  edge *tmp = pop(&small);
  while(tmp!=NULL){
    insert(main, tmp);
  }
}

void  display(struct edge *r)
{
    if(r==NULL)
      return;
    while(r!=NULL)
    {
      printf("%d",r->msg->relation_info.identifier.relation_id.id);
      r=r->next;
      printf("\n");
    }
}

int count(struct edge *n)
{
    int c=0;
    while(n!=NULL)
    {
      n=n->next;
      c++;
    }
    return c;
}

void insert_in_bundle(prov_entry_t *elt){
  int i;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL){
      append(bundle.list[i], elt);
      bundle.last[i]=elt;
      return;
    }else if(edge_greater_than(elt, bundle.last[i])){
      append(bundle.list[i], elt);
      bundle.last[i]=elt;
      return;
    }
  }
}

void merge_bundle() {
  int i;
  for(i=1; i<MAX_BUNDLE; i++){
    if(bundle.last[i]==NULL)
      return;
    merge(&bundle.list[0], bundle.list[i]);
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
