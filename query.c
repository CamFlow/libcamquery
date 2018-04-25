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

struct provenance_query_hooks hooks = {
  .out_edge=&out_edge,
  .in_edge=&in_edge,
  .free=NULL,
  .alloc=NULL,
};

QUERY_DESCRIPTION("An example query");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier");
QUERY_VERSION("0.1");
QUERY_NAME("My Example Query");

register_query(init, hooks);
