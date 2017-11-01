#define SERVICE_QUERY
#include <camquery.h>

static void init( void ){
  print("id,type,in,in_type1,in_type2,in_type3,in_type4,in_type5,in_type6,in_type7,in_type8,in_type9,in_type10,utime,stime,vm,rss,hw_vm,hw_rss,rbytes,wbytes,cancel_wbytes");
}

struct propagate {
  uint32_t in;
  uint64_t in_type[10];
  uint64_t utime;
	uint64_t stime;
  uint64_t vm;
  uint64_t rss;
  uint64_t hw_vm;
  uint64_t hw_rss;
  uint64_t rbytes;
	uint64_t wbytes;
	uint64_t cancel_wbytes;
};

static void print_node(prov_entry_t* node){
  char id[PROV_ID_STR_LEN];
  struct propagate *np = node->msg_info.var_ptr;
  ID_ENCODE(get_prov_identifier(node).buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);
  print("cf:%s,%s,%d,%lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu",
    id,
    node_id_to_str(node_type(node)),
    np->in,
    np->in_type[0],
    np->in_type[1],
    np->in_type[2],
    np->in_type[3],
    np->in_type[4],
    np->in_type[5],
    np->in_type[6],
    np->in_type[7],
    np->in_type[8],
    np->in_type[9],
    np->utime,
    np->stime,
    np->vm,
    np->rss,
    np->hw_vm,
    np->hw_rss,
    np->rbytes,
    np->wbytes,
    np->cancel_wbytes);
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
  if (np->in < 10)
    np->in_type[np->in] = edge_type(edge);
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
