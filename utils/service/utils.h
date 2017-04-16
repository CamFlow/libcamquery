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
#ifndef ___LIB_QUERY_UTILS_H
#define ___LIB_QUERY_UTILS_H
#include <stdio.h>
#include <stdlib.h>

#define	LOG_FILE "/tmp/audit.log"
static FILE *fp=NULL;
static pthread_mutex_t l_log =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

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
#endif
