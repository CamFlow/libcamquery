
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

 /*
 *
 * Author: Thomas Pasquier <tfjmp2@cam.ac.uk>
 *
 * Copyright (C) 2015 University of Cambridge
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
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

#include "provenancelib.h"
#include "provenanceutils.h"
#include "provenancePovJSON.h"

#define	LOG_FILE "/tmp/audit.log"
#define gettid() syscall(SYS_gettid)

static pthread_mutex_t l_log =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

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

bool filter(union prov_msg* msg){
  // do something here
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
