/*
 *
 * Author: Thomas Pasquier <tfjmp@g.harvard.edu>
 *         Michael Han <hanx@g.havard.edu>
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

#define	LOG_FILE "/tmp/audit.log"
#define gettid() syscall(SYS_gettid)
#define WIN_SIZE 100
#define WAIT_TIME 5

static inline struct timespec time_elapsed(struct timespec start, struct timespec end) {
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

static inline int edge_compare(struct hashable_edge* he1, struct hashable_edge* he2) {
  if (relation_identifier(he1->msg).id > relation_identifier(he2->msg).id)
    return 1;
  if (relation_identifier(he2->msg).id == relation_identifier(he2->msg).id)
    return 0;
  return -1;
}

extern static struct hashable_node *node_hash_table;
extern static struct hashable_edge *edge_hash_head;
