#define KERNEL_QUERY
#include "query.h"

static label_t secret;

static void init( void ){
  print("Hello world!");
  secret = generate_label("secret");
}

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  //print("Query: (%llx)--%llx-->", prov_type(node), prov_type(edge));
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
  //print("Query: --%lld", relation_identifier(edge).id);
  if( has_label(edge, secret) ){
    if( node_type(node) == ENT_INODE_SOCKET )
      return CAMFLOW_RAISE_WARNING;
    else
      add_label(node, secret);
  }
  return 0;
}

register_query(init, in_edge, out_edge);
