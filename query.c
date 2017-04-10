#define KERNEL_QUERY
#include "include/camflow_query.h"

static label_t secret;

static void init( void ){
  printf("Hello world!");
  secret = generate_label("secret");
}

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  //printf("Query: (%llx)--%llx-->", prov_type(node), prov_type(edge));
  if( edge_type(edge)== RL_WRITE              ||
      edge_type(edge)== RL_READ               ||
      edge_type(edge)== RL_RCV                ||
      edge_type(edge)== RL_SND                ||
      edge_type(edge)== RL_VERSION            ||
      edge_type(edge)== RL_VERSION_PROCESS    ||
      edge_type(edge)== RL_CLONE){
    if( has_label(node, secret) )
      add_label(edge, secret);
  }
  return 0;
}

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  //printf("Query: --%llx-->(%llx)", prov_type(edge), prov_type(node));
  if( has_label(edge, secret) ){
    if( node_type(node) == ENT_INODE_SOCKET )
      return CAMFLOW_RAISE_WARNING;
    else
      add_label(node, secret);
  }
  return 0;
}

QUERY_DESCRIPTION("An example query");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier");
QUERY_VERSION("0.1");
QUERY_NAME("My Example Query");
register_query(init, in_edge, out_edge);
