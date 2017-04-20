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

void append(struct edge **list, prov_entry_t* msg)
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
      return;
    }
    right=head;
    while(right->next != NULL)
      right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}

void  display(struct edge *r)
{
    if(r==NULL)
      return;
    while(r!=NULL)
    {
      printf("Entry: %d \t %lld\n", relation_identifier(r->msg).boot_id, relation_identifier(r->msg).id);
      r=r->next;
    }
}

void insert(struct edge **list, struct edge *edge){
    struct edge *temp;
    temp=*list;
    printf("Start: %lld\t %d\n", relation_identifier(edge->msg).boot_id, relation_identifier(edge->msg).id);
    display(temp);
    if (temp==NULL) {
      printf("Temp is null\n");
      *list=edge;
      edge->next=NULL;
      return;
    } else if( edge_greater_than(temp->msg, edge->msg) ) {
      printf("New head\n");
      edge->next=temp;
      (*list)=edge;
      return;
    } else {
      while(temp!=NULL) {
          printf("while %d\n", relation_identifier(temp->msg).boot_id);
          if(temp->next==NULL) {
            printf("There\n");
            temp->next = edge;
            edge->next = NULL;
            return;
          } else {
            printf("Here and there\n");
            if( edge_greater_than(temp->next->msg, edge->msg) ) {
              printf("Here\n");
              edge->next = temp->next;
              temp->next = edge;
              return;
            }
            temp = temp->next;
          }
      }
    }
}

struct edge* edge_pop(struct edge **list){
  struct edge *tmp = (*list);
  if(tmp==NULL)
    return NULL;
  (*list) = tmp->next;
  tmp->next=NULL;
  return tmp;
}

void edge_merge(struct edge **main, struct edge *small){
  struct  edge *tmp = edge_pop(&small);
  while(tmp!=NULL){
    printf("\n\n");
    insert(main, tmp);
    printf("Next!\n");
    tmp = edge_pop(&small);
    printf("Popped!\n");
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

void display_bundle(){
  int i;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL){
      printf("List %d is empty\n", i);
      return;
    }
    printf("List %d\n", i);
    display(bundle.list[i]);
  }
}

void insert_in_bundle(prov_entry_t *elt){
  int i;
  for(i=0; i<MAX_BUNDLE; i++){
    if(bundle.list[i]==NULL){
      append(&bundle.list[i], elt);
      bundle.last[i]=elt;
      return;
    }else if(edge_greater_than(elt, bundle.last[i])){
      append(&bundle.list[i], elt);
      bundle.last[i]=elt;
      return;
    }
  }
}

void merge_bundle() {
  int i;
  printf("======BUNDLING======\n");
  printf("====================\n");
  display_bundle();
  for(i=1; i<MAX_BUNDLE; i++){
    if(bundle.last[i]==NULL)
      return;
    edge_merge(&bundle.list[0], bundle.list[i]);
    bundle.list[i]=NULL;
    printf("======BUNDLING %d====\n", i);
    printf("====================\n");
    display_bundle();
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
#endif
