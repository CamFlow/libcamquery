/*
 *
 * Author: Thomas Pasquier <thomas.pasquier@bristol.ac.uk>
 * Author: Xueyuan Han <hanx@g.harvard.edu>
 *
 * Copyright (C) 2017-2018 Harvard University, University of Bristol
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

static int prov_flow(prov_entry_t* from, prov_entry_t* edge, prov_entry_t* to){
  print("From:\t(%s)", node_str(prov_type(from)) );
  print("Edge:\t-%s->", relation_str(prov_type(edge)) );
  print("To:\t(%s)", node_str(prov_type(to)) );
  return 0;
}

struct provenance_query_hooks hooks = {
  .flow=&prov_flow,
  .free=NULL,
  .alloc=NULL,
};

QUERY_DESCRIPTION("An example query");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("John Doe");
QUERY_VERSION("1.0");
QUERY_NAME("My Example Query");

register_query(init, hooks);
