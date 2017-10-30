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

#define MAX_BUNDLE 50

static pthread_mutex_t c_lock_edge = PTHREAD_MUTEX_INITIALIZER;

struct bundle {
  struct edge *list[MAX_BUNDLE];
  struct edge *last[MAX_BUNDLE];
} bundle;

struct edge
{
    prov_entry_t *msg;
    struct timespec t_exist;
    struct edge *next;
};

struct edge* bundle_head(){
  return bundle.list[0];
}

static inline bool edge_greater_than(prov_entry_t *edge1, prov_entry_t *edge2) {
  uint32_t he1_id = edge1->relation_info.identifier.relation_id.id;
  uint32_t he2_id = edge2->relation_info.identifier.relation_id.id;
  uint32_t he1_boot = edge1->relation_info.identifier.relation_id.boot_id;
  uint32_t he2_boot = edge2->relation_info.identifier.relation_id.boot_id;
  if (he1_boot > he2_boot)
    return true;
  else if(he1_boot < he2_boot)
    return false;
  if (he1_id > he2_id)
    return true;
  return false;
}

static inline void append(struct edge **list,  struct edge **last, prov_entry_t* msg)
{
    struct edge *temp,*right;
    struct edge *head;
    head = *list;
    temp= (struct edge *)malloc(sizeof(struct edge));
    clock_gettime(CLOCK_REALTIME, &(temp->t_exist));
    temp->msg = msg;
    if (head == NULL)
    {
      *list=temp;
      (*list)->next=NULL;
      *last=temp;
      return;
    }
    (*last)->next = temp;
    temp->next = NULL;
    (*last) = temp;
}

static inline void  display(struct edge *r)
{
    if(r==NULL)
      return;
    while(r!=NULL)
    {
      printf("Entry: %d \t %ld\n", relation_identifier(r->msg).boot_id, relation_identifier(r->msg).id);
      r=r->next;
    }
}

static inline void move_node(struct edge **dest, struct edge **src){
  struct edge *n = *src;
  if(n==NULL)
    return;
  *src = n->next;
  n->next = *dest;
  *dest = n;
}

static inline struct edge* sorted_merge(struct edge *a, struct edge *b){
    struct edge *result = NULL;
    struct edge **last = &result;

    while(1){
      if(a==NULL){
        *last=b;
        break;
      }else if (b==NULL){
        *last=a;
        break;
      }
      if(edge_greater_than(b->msg, a->msg)){
        move_node(last, &a);
      }else{
        move_node(last, &b);
      }
      last = &((*last)->next);
    }
    return result;
}

struct edge* edge_pop(struct edge **list, struct edge **last){
  struct edge *tmp = (*list);
  if(tmp==NULL)
    return NULL;
  (*list) = tmp->next;
  if(*list==NULL)
    *last = NULL;
  tmp->next=NULL;
  return tmp;
}

/*void edge_merge(struct edge **main, struct edge *small){
  struct  edge *tmp = edge_pop(&small);
  while(tmp!=NULL){
    insert(main, tmp);
    tmp = edge_pop(&small);
  }
}*/

static inline int count(struct edge *n)
{
    int c=0;
    while(n!=NULL)
    {
      n=n->next;
      c++;
    }
    return c;
}

static inline void display_bundle(){
  int i;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL){
      return;
    }
    display(bundle.list[i]);
  }
}

static inline void insert_in_bundle(prov_entry_t *elt){
  int i;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL){
      append(&bundle.list[i], &bundle.last[i], elt);
      return;
    }else if(edge_greater_than(elt, bundle.last[i]->msg)){
      append(&bundle.list[i], &bundle.last[i], elt);
      return;
    }
  }
}

static inline void merge_bundle() {
  int i;
  for(i=1; i<MAX_BUNDLE; i++){
    if(bundle.last[i]==NULL)
      return;
    bundle.list[0] = sorted_merge(bundle.list[0], bundle.list[i]);
    bundle.list[i]= NULL;
  }
}

static inline int insert_edge(prov_entry_t *elt){
  pthread_mutex_lock(&c_lock_edge);
  insert_in_bundle(elt);
  pthread_mutex_unlock(&c_lock_edge);
}

static inline uint32_t edge_count_nolock(void){
  int i;
  uint32_t c=0;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL)
      return c;
    c+=count(bundle.list[i]);
  }
  return c;
}

static inline uint32_t bundle_count_nolock(void){
  int i;
  uint32_t c=0;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL)
      return c;
    c++;
  }
  return c;
}
#endif
