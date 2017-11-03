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
#define BASE_FOLDER "/tmp/"

struct vec {
  uint32_t in;
  uint64_t in_type[MAX_PARENT];
  uint64_t p_type[MAX_PARENT];
  int64_t offset[MAX_PARENT];
  uint64_t utime;
	uint64_t stime;
  uint64_t vm;
  uint64_t rss;
  uint64_t hw_vm;
  uint64_t hw_rss;
  uint64_t rbytes;
	uint64_t wbytes;
	uint64_t cancel_wbytes;
  uint32_t uid;
  uint32_t gid;
  bool recorded;
};

#define sappend(buffer, fmt, ...) sprintf(buffer, "%s"fmt, buffer, ##__VA_ARGS__)

static inline void write_vector(prov_entry_t* node, void (*specific)(char*, prov_entry_t*, struct vec*), int *fd, const char filename[]){
  int i;
  struct vec *np = node->msg_info.var_ptr;
  char id[PATH_MAX];
  memset(id, 0, PATH_MAX);
  char buffer[4096];

  if (*fd == 0)
    *fd = open(filename, O_WRONLY|O_APPEND);

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

static int taskfd=0;
static void __write_task(char *buffer, prov_entry_t* node, struct vec* np){
  sappend(buffer, "%lu,", np->utime);
  sappend(buffer, "%lu,", np->stime);
  sappend(buffer, "%lu,", np->vm);
  sappend(buffer, "%lu,", np->rss);
  sappend(buffer, "%lu,", np->hw_vm);
  sappend(buffer, "%lu,", np->hw_rss);
  sappend(buffer, "%lu,", np->rbytes);
  sappend(buffer, "%lu,", np->wbytes);
  sappend(buffer, "%lu,", np->cancel_wbytes);
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
