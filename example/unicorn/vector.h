/*
 *
 * Author: Thomas Pasquier <tfjmp@g.harvard.edu>
 * Author: Xueyuan Han <hanx@g.harvard.edu>
 *
 * Copyright (C) 2017 Harvard University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 */
#include <camquery.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_PARENT 2
#define MAX_DEPTH  3
#define BASE_FOLDER "/tmp/"

struct vec {
  uint32_t in;
  uint64_t in_type[MAX_PARENT];
  uint64_t p_type[MAX_PARENT];
  int64_t offset[MAX_PARENT];
  uint64_t utime[MAX_DEPTH];
	uint64_t stime[MAX_DEPTH];
  uint64_t vm[MAX_DEPTH];
  uint64_t rss[MAX_DEPTH];
  uint64_t hw_vm[MAX_DEPTH];
  uint64_t hw_rss[MAX_DEPTH];
  uint64_t rbytes[MAX_DEPTH];
	uint64_t wbytes[MAX_DEPTH];
	uint64_t cancel_wbytes[MAX_DEPTH];
  uint32_t uid;
  uint32_t gid;
  bool recorded;
};

#define sappend(buffer, fmt, ...) sprintf(buffer, "%s"fmt, buffer, ##__VA_ARGS__)

#define iterative_header(nb, name) for(i=0; i<nb; i++) sappend(buffer, name"_%d,", i)

static inline void write_vector(prov_entry_t* node, void (*specific)(char*, prov_entry_t*, struct vec*), int *fd, const char filename[]){
  int i;
  struct vec *np = node->msg_info.var_ptr;
  char id[PATH_MAX];
  memset(id, 0, PATH_MAX);
  char buffer[4096];

  if (np->recorded)
    return;


  ID_ENCODE(get_prov_identifier(node).buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);
  sprintf(buffer, "cf:%s,", id);
  sappend(buffer, "%s,", node_id_to_str(node_type(node)));
  sappend(buffer, "%u,", np->in);
  for(i=0; i<MAX_PARENT; i++){
    if(np->in_type[i] != 0)
      sappend(buffer, "%s,", relation_id_to_str(np->in_type[i]));
    else
      sappend(buffer, "NULL,");
  }
  for(i=0; i<MAX_PARENT; i++){
    if(np->p_type[i] != 0)
      sappend(buffer, "%s,", node_id_to_str(np->p_type[i]));
    else
      sappend(buffer, "NULL,");
  }
  specific(buffer, node, np);

  sappend(buffer, "\n");

  // write down buffer
  if ( write(*fd, buffer, strlen(buffer)) < 0 )
    log_error("Failed writting vector");
  np->recorded=true;
}

#define declare_writer(fcn_name, specific, file, filename) static inline void fcn_name(prov_entry_t* node){\
  write_vector(node, specific, &file, filename);\
}\

#define output_task_evol(param) for(i=0; i<MAX_DEPTH; i++) sappend(buffer, "%lu,",  node->task_info.param - np->param[i])

static int taskfd=0;
static void __write_task(char *buffer, prov_entry_t* node, struct vec* np){
  int i;
  output_task_evol(utime);
  output_task_evol(stime);
  output_task_evol(vm);
  output_task_evol(rss);
  output_task_evol(hw_vm);
  output_task_evol(hw_rss);
  output_task_evol(rbytes);
  output_task_evol(wbytes);
  output_task_evol(cancel_wbytes);
  sappend(buffer, "%u,", node->task_info.utsns);
  sappend(buffer, "%u,", node->task_info.ipcns);
  sappend(buffer, "%u,", node->task_info.mntns);
  sappend(buffer, "%u,", node->task_info.pidns);
  sappend(buffer, "%u,", node->task_info.netns);
  sappend(buffer, "%u,", node->task_info.cgroupns);
  sappend(buffer, "%u,", node->msg_info.uid);
  sappend(buffer, "%u,", node->msg_info.gid);
}
declare_writer(write_task, __write_task, taskfd, BASE_FOLDER "task");

static inline void task_header(void) {
  int i;
  char buffer[4096];

  sprintf(buffer, "id,");
  sappend(buffer, "type,");
  iterative_header(MAX_DEPTH, "in_type");
  iterative_header(MAX_DEPTH, "a_type");
  iterative_header(MAX_DEPTH, "utime");
  iterative_header(MAX_DEPTH, "stime");
  iterative_header(MAX_DEPTH, "vm");
  iterative_header(MAX_DEPTH, "rss");
  iterative_header(MAX_DEPTH, "hw_vm");
  iterative_header(MAX_DEPTH, "hw_rss");
  iterative_header(MAX_DEPTH, "rbytes");
  iterative_header(MAX_DEPTH, "wbytes");
  iterative_header(MAX_DEPTH, "cancel_wbytes");
  sappend(buffer, "uid,");
  sappend(buffer, "gid,");
  sappend(buffer, "\n");
  if ( write(taskfd, buffer, strlen(buffer)) < 0 )
    log_error("Failed writting task header");
}

static inline void init_files( void ){
  taskfd = open(BASE_FOLDER "task", O_WRONLY|O_APPEND);
  task_header();
}
