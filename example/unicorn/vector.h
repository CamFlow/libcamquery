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

#define CALC(n) ((n) <= 0 ? 0 : \
                 (n) == 1 ? 2 : \
                 (n) == 2 ? 6 : \
                 (n) == 3 ? 14 : \
                 (n) == 4 ? 30 : \
                 (n) == 5 ? 62 : \
                 (n) == 6 ? 126 : \
                 (n) == 7 ? 254 : \
                 (n) == 8 ? 510 : INT_MAX)


#define MAX_PARENT 2
#define MAX_DEPTH  3
#define REC_SIZE CALC(MAX_DEPTH)
#define BASE_FOLDER "/tmp/"
#define TASK_FILE BASE_FOLDER"task"
#define FILE_FILE BASE_FOLDER"file"


struct vec {
  uint32_t in;
  uint64_t in_type[REC_SIZE];
  uint64_t in_node;
  uint64_t p_type[REC_SIZE];
  uint64_t utime[MAX_DEPTH];
	uint64_t stime[MAX_DEPTH];
  uint64_t vm[MAX_DEPTH];
  uint64_t rss[MAX_DEPTH];
  uint64_t hw_vm[MAX_DEPTH];
  uint64_t hw_rss[MAX_DEPTH];
  uint64_t rbytes[MAX_DEPTH];
	uint64_t wbytes[MAX_DEPTH];
	uint64_t cancel_wbytes[MAX_DEPTH];
  uint64_t utsns[MAX_DEPTH];
  uint64_t ipcns[MAX_DEPTH];
  uint64_t mntns[MAX_DEPTH];
  uint64_t pidns[MAX_DEPTH];
  uint64_t netns[MAX_DEPTH];
  uint64_t cgroupns[MAX_DEPTH];
  uint64_t uid;
  uint64_t p_uid[REC_SIZE];
  uint64_t gid;
  uint64_t p_gid[REC_SIZE];
  uint16_t mode[MAX_DEPTH];
  bool recorded;
};

#define sappend(buffer, fmt, ...) sprintf(buffer, "%s"fmt, buffer, ##__VA_ARGS__)

static inline void write_vector(prov_entry_t* node, void (*specific)(char*, prov_entry_t*, struct vec*), int *fd, const char filename[]){
  int i;
  struct vec *np = node->msg_info.var_ptr;
  char id[PATH_MAX];
  memset(id, 0, PATH_MAX);
  char buffer[4096];

  if (np->recorded)
    return;

  // record node ID
  ID_ENCODE(get_prov_identifier(node).buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);
  sprintf(buffer, "cf:%s,", id);

  // record edge type
  for(i=0; i<REC_SIZE; i++){
    if(np->in_type[i] != 0)
      sappend(buffer, "%s,", relation_id_to_str(np->in_type[i]));
    else
      sappend(buffer, "NULL,");
  }

  // record node type
  sappend(buffer, "%s,", node_id_to_str(node_type(node)));
  for(i=0; i<REC_SIZE; i++){
    if(np->p_type[i] != 0)
      sappend(buffer, "%s,", node_id_to_str(np->p_type[i]));
    else
      sappend(buffer, "NULL,");
  }

  // user id
  if (prov_has_uidgid(np->p_type[i]))
    sappend(buffer, "%u,", node->msg_info.uid);
  else
    sappend(buffer, "NULL,");
  for(i=0; i<REC_SIZE; i++){
    if (prov_has_uidgid(np->p_type[i]))
      sappend(buffer, "%d,", (uint32_t)np->p_uid[i]);
    else
      sappend(buffer, "NULL,");
  }

  // group id
  if (prov_has_uidgid(np->p_type[i]))
    sappend(buffer, "%u,", node->msg_info.gid);
  else
    sappend(buffer, "NULL,");
  for(i=0; i<REC_SIZE; i++){
    if (prov_has_uidgid(np->p_type[i]))
      sappend(buffer, "%d,", (uint32_t)np->p_gid[i]);
    else
      sappend(buffer, "NULL,");
  }

  // record information specific to a give node type
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
#define output_task_stat(param) for(i=0; i<MAX_DEPTH; i++) sappend(buffer, "%lu,",  np->param[i])

static void __write_task(char *buffer, prov_entry_t* node, struct vec* np){
  int i;

  sappend(buffer, "%lu,",  node->task_info.utime);
  output_task_stat(utime);
  sappend(buffer, "%lu,",  node->task_info.stime);
  output_task_stat(stime);
  sappend(buffer, "%lu,",  node->task_info.vm);
  output_task_stat(vm);
  sappend(buffer, "%lu,",  node->task_info.rss);
  output_task_stat(rss);
  sappend(buffer, "%lu,",  node->task_info.hw_vm);
  output_task_stat(hw_vm);
  sappend(buffer, "%lu,",  node->task_info.hw_rss);
  output_task_stat(hw_rss);
  sappend(buffer, "%lu,",  node->task_info.rbytes);
  output_task_stat(rbytes);
  sappend(buffer, "%lu,",  node->task_info.wbytes);
  output_task_stat(wbytes);
  sappend(buffer, "%lu,",  node->task_info.cancel_wbytes);
  output_task_stat(cancel_wbytes);
  sappend(buffer, "%u,", node->task_info.utsns);
  output_task_stat(utsns);
  sappend(buffer, "%u,", node->task_info.ipcns);
  output_task_stat(ipcns);
  sappend(buffer, "%u,", node->task_info.mntns);
  output_task_stat(mntns);
  sappend(buffer, "%u,", node->task_info.pidns);
  output_task_stat(pidns);
  sappend(buffer, "%u,", node->task_info.netns);
  output_task_stat(netns);
  sappend(buffer, "%u,", node->task_info.cgroupns);
  output_task_stat(cgroupns);
}
static int taskfd=0;
declare_writer(write_task, __write_task, taskfd, TASK_FILE);

#define output_mode_stat(param) for(i=0; i<MAX_DEPTH; i++) sappend(buffer, "%x,",  np->param[i])
static void __inode_task(char *buffer, prov_entry_t* node, struct vec* np){
  int i;

  sappend(buffer, "%x,", node->inode_info.mode);
  output_mode_stat(mode);
}
static int filefd=0;
declare_writer(write_file, __inode_task, filefd, FILE_FILE);

static inline void init_files( void ){
  taskfd = open(TASK_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
  filefd = open(FILE_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
}
