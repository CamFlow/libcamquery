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
#define SOCKET_FILE BASE_FOLDER"socket"
#define FIFO_FILE BASE_FOLDER"fifo"
#define LINK_FILE BASE_FOLDER"link"
#define MMAP_FILE BASE_FOLDER"mmap"

struct vec {
  uint32_t in;
  uint64_t in_type;
  uint64_t p_in_type[REC_SIZE];
  uint64_t type;
  uint64_t p_type[REC_SIZE];
  uint64_t utime;
  uint64_t p_utime[REC_SIZE];
  uint64_t stime;
	uint64_t p_stime[REC_SIZE];
  uint64_t vm;
  uint64_t p_vm[REC_SIZE];
  uint64_t rss;
  uint64_t p_rss[REC_SIZE];
  uint64_t hw_vm;
  uint64_t p_hw_vm[REC_SIZE];
  uint64_t hw_rss;
  uint64_t p_hw_rss[REC_SIZE];
  uint64_t rbytes;
  uint64_t p_rbytes[REC_SIZE];
  uint64_t wbytes;
	uint64_t p_wbytes[REC_SIZE];
  uint64_t cancel_wbytes;
	uint64_t p_cancel_wbytes[REC_SIZE];
  uint32_t utsns;
  uint32_t p_utsns[REC_SIZE];
  uint32_t ipcns;
  uint32_t p_ipcns[REC_SIZE];
  uint32_t mntns;
  uint32_t p_mntns[REC_SIZE];
  uint32_t pidns;
  uint32_t p_pidns[REC_SIZE];
  uint32_t netns;
  uint32_t p_netns[REC_SIZE];
  uint32_t cgroupns;
  uint32_t p_cgroupns[REC_SIZE];
  uint64_t uid;
  uint64_t p_uid[REC_SIZE];
  uint64_t gid;
  uint64_t p_gid[REC_SIZE];
  uint16_t mode;
  uint16_t p_mode[REC_SIZE];
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
    if(np->p_in_type[i] != 0)
      sappend(buffer, "%s,", relation_id_to_str(np->p_in_type[i]));
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

#define task_value_tree(fmt, param)  sappend(buffer, fmt,  node->task_info.param);\
                                for(i=0; i<REC_SIZE; i++){\
                                  if(np->p_type[i] == ACT_TASK)\
                                     sappend(buffer, fmt, np->p_ ## param[i]);\
                                  else\
                                    sappend(buffer, "NULL,");\
                                }

static void __write_task(char *buffer, prov_entry_t* node, struct vec* np){
  int i;

  task_value_tree("%lu,", utime);
  task_value_tree("%lu,", stime);
  task_value_tree("%lu,", vm);
  task_value_tree("%lu,", rss);
  task_value_tree("%lu,", hw_vm);
  task_value_tree("%lu,", hw_rss);
  task_value_tree("%lu,", rbytes);
  task_value_tree("%lu,", wbytes);
  task_value_tree("%lu,", cancel_wbytes);
  task_value_tree("%u,", utsns);
  task_value_tree("%u,", ipcns);
  task_value_tree("%u,", mntns);
  task_value_tree("%u,", pidns);
  task_value_tree("%u,", netns);
  task_value_tree("%u,", cgroupns);
}
static int taskfd=0;
declare_writer(write_task, __write_task, taskfd, TASK_FILE);

#define inode_value_tree(fmt, param)  sappend(buffer, fmt,  node->inode_info.param);\
                                for(i=0; i<REC_SIZE; i++){\
                                  if(prov_is_inode(np->p_type[i]))\
                                     sappend(buffer, fmt, np->p_ ## param[i]);\
                                  else\
                                    sappend(buffer, "NULL,");\
                                }

static void __write_inode(char *buffer, prov_entry_t* node, struct vec* np){
  int i;

  inode_value_tree("%x,", mode);
}
static int filefd=0;
declare_writer(write_file, __write_inode, filefd, FILE_FILE);
static int socketfd=0;
declare_writer(write_socket, __write_inode, socketfd, SOCKET_FILE);
static int fifofd=0;
declare_writer(write_fifo, __write_inode, fifofd, FIFO_FILE);
static int linkfd=0;
declare_writer(write_link, __write_inode, linkfd, LINK_FILE);
static int mmapfd=0;
declare_writer(write_mmap, __write_inode, mmapfd, MMAP_FILE);

static inline void init_files( void ){
  taskfd = open(TASK_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
  filefd = open(FILE_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
  socketfd = open(SOCKET_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
  fifofd = open(FIFO_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
  linkfd = open(LINK_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
  mmapfd = open(MMAP_FILE, O_WRONLY|O_APPEND|O_CREAT, 0x0644);
}
