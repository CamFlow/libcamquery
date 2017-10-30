#define SERVICE_QUERY
#include "include/camquery.h"

static label_t secret;

static void init( void ){
  print("Hello world!");
  secret = generate_label("secret");
}

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  print("Out:\t(%s)", node_str(prov_type(node)) );
  print("Out:\t-%s->", relation_str(prov_type(edge)) );
  return 0;
}

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  print("In:\t-%s->", relation_str(prov_type(edge)) );
  print("In:\t(%s)", node_str(prov_type(node)) );
  return 0;
}

QUERY_DESCRIPTION("An example query");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier");
QUERY_VERSION("0.1");
QUERY_NAME("My Example Query");

// service specific build settings
#ifdef SERVICE_QUERY
QUERY_CHANNEL("example");
QUERY_OUTPUT("/tmp/example.log");
#endif

register_query(init, in_edge, out_edge);
