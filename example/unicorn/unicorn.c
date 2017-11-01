#define SERVICE_QUERY
#include <camquery.h>

#define MAX_SUPPORTED_PARENT 20

FILE *logfile;

static void init( void ){
  print("id,type,in,in_type1,in_type2,in_type3,in_type4,in_type5,in_type6,in_type7,in_type8,in_type9,in_type10,utime,stime,vm,rss,hw_vm,hw_rss,rbytes,wbytes,cancel_wbytes,utsns,ipcns,mntns,pidns,netns,cgroupns");
  logfile = fopen("/tmp/unicorn", "a");
}

struct propagate {
  uint32_t in;
  uint64_t in_type[MAX_SUPPORTED_PARENT];
  int64_t offset[MAX_SUPPORTED_PARENT];
  uint64_t utime;
	uint64_t stime;
  uint64_t vm;
  uint64_t rss;
  uint64_t hw_vm;
  uint64_t hw_rss;
  uint64_t rbytes;
	uint64_t wbytes;
	uint64_t cancel_wbytes;
  bool recorded;
};

static void print_node(prov_entry_t* node){
  char id[PROV_ID_STR_LEN];
  int i;
  struct propagate *np = node->msg_info.var_ptr;

  // already saved it
  if(np->recorded)
    return;

  ID_ENCODE(get_prov_identifier(node).buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);

  fprintf(logfile, "cf:%s,", id);
  fprintf(logfile, "%s,", node_id_to_str(node_type(node)));
  fprintf(logfile, "%u,", np->in);
  for(i=0; i<MAX_SUPPORTED_PARENT; i++){
    if(np->in_type[i]!=0)
      fprintf(logfile, "%s,", relation_id_to_str(np->in_type[i]));
    else
      fprintf(logfile, "NULL,");
  }
  for(i=0; i<MAX_SUPPORTED_PARENT; i++){
    fprintf(logfile, "%ld,", np->offset[i]);
  }
  fprintf(logfile, "%lu,", np->utime);
  fprintf(logfile, "%lu,", np->stime);
  fprintf(logfile, "%lu,", np->vm);
  fprintf(logfile, "%lu,", np->rss);
  fprintf(logfile, "%lu,", np->hw_vm);
  fprintf(logfile, "%lu,", np->hw_rss);
  fprintf(logfile, "%lu,", np->rbytes);
  fprintf(logfile, "%lu,", np->wbytes);
  fprintf(logfile, "%lu,", np->cancel_wbytes);
  fprintf(logfile, "\n");
  fflush(logfile);
  np->recorded=true;
}

void* zalloc(size_t size){
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  struct propagate *ep;
  struct propagate *np;

  if (edge->msg_info.var_ptr == NULL)
    edge->msg_info.var_ptr = zalloc(sizeof(struct propagate));
  ep = edge->msg_info.var_ptr;

  if (node->msg_info.var_ptr == NULL)
    node->msg_info.var_ptr = zalloc(sizeof(struct propagate));
  np = node->msg_info.var_ptr;

  if (edge_type(edge) == RL_VERSION_PROCESS) {
    ep->utime = node->task_info.utime;
    ep->stime = node->task_info.stime;
    ep->vm = node->task_info.vm;
    ep->rss = node->task_info.rss;
    ep->hw_vm = node->task_info.hw_vm;
    ep->hw_rss = node->task_info.hw_rss;
    ep->rbytes = node->task_info.rbytes;
    ep->wbytes = node->task_info.wbytes;
    ep->cancel_wbytes = node->task_info.cancel_wbytes;
  }
  print_node(node);
  return 0;
}

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  struct propagate *ep;
  struct propagate *np;

  ep = edge->msg_info.var_ptr;

  if (node->msg_info.var_ptr == NULL)
    node->msg_info.var_ptr = zalloc(sizeof(struct propagate));
  np = node->msg_info.var_ptr;

  /* couting in edges */
  if (np->in < MAX_SUPPORTED_PARENT) {
    np->in_type[np->in] = edge_type(edge);
    np->offset[np->in] = edge->relation_info.offset;
  }
  np->in++;

  if (edge_type(edge) == RL_VERSION_PROCESS) {
    np->utime = node->task_info.utime - ep->utime;
    np->stime = node->task_info.stime - ep->stime;
    np->vm = node->task_info.vm - ep->vm;
    np->rss = node->task_info.rss - ep->rss;
    np->hw_vm = node->task_info.hw_vm - ep->hw_vm;
    np->hw_rss = node->task_info.hw_rss - ep->hw_rss;
    np->rbytes = node->task_info.rbytes - ep->rbytes;
    np->wbytes = node->task_info.wbytes - ep->wbytes;
    np->cancel_wbytes = node->task_info.cancel_wbytes - ep->cancel_wbytes;
  }
  return 0;
}

QUERY_DESCRIPTION("Compute feature vectors for unicorn!");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier and Michael Han");
QUERY_VERSION("0.1");
QUERY_NAME("Unicorn");

// service specific build settings
#ifdef SERVICE_QUERY
QUERY_CHANNEL("unicorn");
QUERY_OUTPUT("/tmp/unicorn.log");
#endif

register_query(init, in_edge, out_edge);
