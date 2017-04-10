#define KERNEL_QUERY
#include "include/camflow_query.h"

static label_t secret;

static void init( void ){
  printf("Hello world!");
  secret = generate_label("secret");
}

static inline void compare_buffer(uint8_t* a, uint8_t* b){
  int i;
  for(i=0; i<PROV_IDENTIFIER_BUFFER_LENGTH;i++){
    if(a[i]!=b[i]){
      printf("=====ERROR IN BUFFER======\n\n");
    }
  }
}

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  printf("\n\n=====NEW RELATION======");
  printf("=====%llx======", edge->relation_info.identifier.relation_id.type);
  compare_buffer(node->node_info.identifier.buffer, edge->relation_info.snd.buffer);
  printf("Query: type\t (%llx)--%llx-->",
    node->node_info.identifier.node_id.type,
    edge->relation_info.snd.node_id.type);
  if(node->node_info.identifier.node_id.type!=edge->relation_info.snd.node_id.type)
    printf("=====ERROR======\n\n");

  printf("Query: id\t (%llx)--%llx-->",
    node->node_info.identifier.node_id.id,
    edge->relation_info.snd.node_id.id);
  if(node->node_info.identifier.node_id.id!=edge->relation_info.snd.node_id.id)
    printf("=====ERROR======\n\n");

  printf("Query: boot_id\t (%x)--%x-->",
    node->node_info.identifier.node_id.boot_id,
    edge->relation_info.snd.node_id.boot_id);
  if(node->node_info.identifier.node_id.boot_id!=edge->relation_info.snd.node_id.boot_id)
    printf("=====ERROR======\n\n");

  printf("Query: machine_id\t (%x)--%x-->",
    node->node_info.identifier.node_id.machine_id,
    edge->relation_info.snd.node_id.machine_id);
  if(node->node_info.identifier.node_id.machine_id!=edge->relation_info.snd.node_id.machine_id)
    printf("=====ERROR======\n\n");

  printf("Query: version\t (%x)--%x-->",
    node->node_info.identifier.node_id.version,
    edge->relation_info.snd.node_id.version);
  if(node->node_info.identifier.node_id.version!=edge->relation_info.snd.node_id.version)
    printf("=====ERROR======\n\n");
  return 0;
}

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  compare_buffer(node->node_info.identifier.buffer, edge->relation_info.rcv.buffer);
  printf("Query: type\t --%llx-->(%llx)",
    edge->relation_info.rcv.node_id.type,
    node->node_info.identifier.node_id.type);
  if(node->node_info.identifier.node_id.type!=edge->relation_info.rcv.node_id.type)
    printf("=====ERROR======\n\n");

  printf("Query: id\t --%llx-->(%llx)",
    edge->relation_info.rcv.node_id.id,
    node->node_info.identifier.node_id.id);
  if(node->node_info.identifier.node_id.id!=edge->relation_info.rcv.node_id.id)
    printf("=====ERROR======\n\n");

  printf("Query: boot_id\t --%x-->(%x)",
    edge->relation_info.rcv.node_id.boot_id,
    node->node_info.identifier.node_id.boot_id);
  if(node->node_info.identifier.node_id.boot_id!=edge->relation_info.rcv.node_id.boot_id)
    printf("=====ERROR======\n\n");

  printf("Query: machine_id\t --%x-->(%x)",
    edge->relation_info.rcv.node_id.machine_id,
    node->node_info.identifier.node_id.machine_id);
  if(node->node_info.identifier.node_id.machine_id!=edge->relation_info.rcv.node_id.machine_id)
    printf("=====ERROR======\n\n");

  printf("Query: version\t --%x-->(%x)",
    edge->relation_info.rcv.node_id.version,
    node->node_info.identifier.node_id.version);
  if(node->node_info.identifier.node_id.version!=edge->relation_info.rcv.node_id.version)
    printf("=====ERROR======\n\n");
  printf("=====END RELATION======\n\n");
  return 0;
}

QUERY_DESCRIPTION("An example query");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier");
QUERY_VERSION("0.1");
QUERY_NAME("My Example Query");
register_query(init, in_edge, out_edge);
