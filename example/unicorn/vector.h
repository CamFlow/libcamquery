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
#include <pwd.h>
#include <grp.h>

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

static inline void write_vector(prov_entry_t* node, void (*specific)(FILE *, prov_entry_t*, struct vec*), FILE* file, const char filename[]){
  int i;
  struct vec *np = node->msg_info.var_ptr;
  char id[PROV_ID_STR_LEN];

  if (file == NULL)
    file = fopen(filename, "a");

  if (np->recorded)
    return;

  ID_ENCODE(get_prov_identifier(node).buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);
  fprintf(file, "cf:%s,", id);
  fprintf(file, "%s,", node_id_to_str(node_type(node)));
  fprintf(file, "%u,", np->in);
  for(i=0; i<MAX_PARENT; i++){
    if(np->in_type[i] != 0)
      fprintf(file, "%s,", relation_id_to_str(np->in_type[i]));
    else
      fprintf(file, "NULL,");
  }
  for(i=0; i<MAX_PARENT; i++){
    if(np->p_type[i] != 0)
      fprintf(file, "%s,", node_id_to_str(np->p_type[i]));
    else
      fprintf(file, "NULL,");
  }

  specific(file, node, np);

  fprintf(file, "\n");
  fflush(file);
  np->recorded=true;
}

#define declare_writer(fcn_name, specific, file, filename) static inline void fcn_name(prov_entry_t* node){\
  write_vector(node, specific, file, filename);\
}\

FILE *taskfile=NULL;
static void __write_task(FILE *file, prov_entry_t* node, struct vec* np){
  struct passwd* pwd;
  struct group* grp;
  fprintf(file, "%lu,", np->utime);
  fprintf(file, "%lu,", np->stime);
  fprintf(file, "%lu,", np->vm);
  fprintf(file, "%lu,", np->rss);
  fprintf(file, "%lu,", np->hw_vm);
  fprintf(file, "%lu,", np->hw_rss);
  fprintf(file, "%lu,", np->rbytes);
  fprintf(file, "%lu,", np->wbytes);
  fprintf(file, "%lu,", np->cancel_wbytes);
  fprintf(file, "%u,", node->task_info.utsns);
  fprintf(file, "%u,", node->task_info.ipcns);
  fprintf(file, "%u,", node->task_info.mntns);
  fprintf(file, "%u,", node->task_info.pidns);
  fprintf(file, "%u,", node->task_info.netns);
  fprintf(file, "%u,", node->task_info.cgroupns);
  pwd = getpwuid(node->msg_info.uid);
  fprintf(file, "%s,", pwd->pw_name);
  grp = getgrgid(node->msg_info.gid);
  fprintf(file, "%s,", grp->gr_name);
}
declare_writer(write_task, __write_task, taskfile, BASE_FOLDER "task");
