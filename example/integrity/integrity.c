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
#define SERVICE_QUERY
#include <camquery.h>

static void init( void ){
  print("integrityd ready!");
}

static int out_edge(prov_entry_t* node, prov_entry_t* edge){
  return 0;
}

static int in_edge(prov_entry_t* edge, prov_entry_t* node){
  return 0;
}

QUERY_DESCRIPTION("Verify graph integrity from userspace");
QUERY_LICSENSE("GPL");
QUERY_AUTHOR("Thomas Pasquier and Michael Han");
QUERY_VERSION("0.1");
QUERY_NAME("Integrity");

// service specific build settings
#ifdef SERVICE_QUERY
QUERY_CHANNEL("integrity");
QUERY_OUTPUT("/tmp/integrity.log");
#endif

register_query(init, in_edge, out_edge);
