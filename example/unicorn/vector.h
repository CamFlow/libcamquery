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
  sprintf(buffer, "%s%s,", buffer, node_id_to_str(node_type(node)));
  sprintf(buffer, "%s%u,", buffer, np->in);
  for(i=0; i<MAX_PARENT; i++){
    if(np->in_type[i] != 0)
      sprintf(buffer, "%s%s,", buffer, relation_id_to_str(np->in_type[i]));
    else
      sprintf(buffer, "%sNULL,", buffer);
  }
  for(i=0; i<MAX_PARENT; i++){
    if(np->p_type[i] != 0)
      sprintf(buffer, "%s%s,", buffer, node_id_to_str(np->p_type[i]));
    else
      sprintf(buffer, "%sNULL,", buffer);
  }

  specific(buffer, node, np);

  sprintf(buffer, "%s\n", buffer);

  write(*fd, buffer, strlen(buffer));
  np->recorded=true;
}

#define declare_writer(fcn_name, specific, file, filename) static inline void fcn_name(prov_entry_t* node){\
  write_vector(node, specific, &file, filename);\
}\

static int taskfd=0;
static void __write_task(char *buffer, prov_entry_t* node, struct vec* np){
  sprintf(buffer, "%s%lu,", buffer, np->utime);
  sprintf(buffer, "%s%lu,", buffer, np->stime);
  sprintf(buffer, "%s%lu,", buffer, np->vm);
  sprintf(buffer, "%s%lu,", buffer, np->rss);
  sprintf(buffer, "%s%lu,", buffer, np->hw_vm);
  sprintf(buffer, "%s%lu,", buffer, np->hw_rss);
  sprintf(buffer, "%s%lu,", buffer, np->rbytes);
  sprintf(buffer, "%s%lu,", buffer, np->wbytes);
  sprintf(buffer, "%s%lu,", buffer, np->cancel_wbytes);
  sprintf(buffer, "%s%u,", buffer, node->task_info.utsns);
  sprintf(buffer, "%s%u,", buffer, node->task_info.ipcns);
  sprintf(buffer, "%s%u,", buffer, node->task_info.mntns);
  sprintf(buffer, "%s%u,", buffer, node->task_info.pidns);
  sprintf(buffer, "%s%u,", buffer, node->task_info.netns);
  sprintf(buffer, "%s%u,", buffer, node->task_info.cgroupns);
  sprintf(buffer, "%s%u,", buffer, node->msg_info.uid);
  sprintf(buffer, "%s%u,", buffer, node->msg_info.gid);
}
declare_writer(write_task, __write_task, taskfd, BASE_FOLDER "task");
