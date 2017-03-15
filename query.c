#define KERNEL_QUERY
#include "include/camflow_query.h"

static tag_t secret;

static void init( void ){
  printf("Hello world!");
  secret = generate_tag("secret");
}

static int out_edge(union prov_msg* node, union prov_msg* edge){
  if( edge_type(edge)== RL_WRITE              ||
      edge_type(edge)== RL_READ               ||
      edge_type(edge)== RL_RCV                ||
      edge_type(edge)== RL_SND                ||
      edge_type(edge)== RL_VERSION            ||
      edge_type(edge)== RL_VERSION_PROCESS    ||
      edge_type(edge)== RL_CLONE){
    if( has_tag(node, secret) )
      add_tag(edge, secret);
  }
  return 0;
}

static int in_edge(union prov_msg* edge, union prov_msg* node){
  if( has_tag(edge, secret) ){
    add_tag(node, secret);
    if( node_type(node) == ENT_INODE_SOCKET )
      return CAMFLOW_RAISE_WARNING;
  }
  return 0;
}

QUERY_DESCRIPTION("An example query");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier");
QUERY_VERSION("0.1");
QUERY_NAME("My Example Query");
register_query(init, in_edge, out_edge);
