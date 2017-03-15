/*
 *
 * Author: Thomas Pasquier <tfjmp@g.harvard.edu>
 *
 * Copyright (C) 2017 Harvard University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 */

#ifdef KERNEL_QUERY

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/camflow_query.h>

typedef uint64_t tag_t;

#define assign_tag(name, str) name = generate_tag(str)

#define register_query(init_fcn, in_fcn, out_fcn)\
struct provenance_query_hooks hooks = {\
  QUERY_HOOK_INIT(out_edge, in_fcn),\
  QUERY_HOOK_INIT(in_edge, out_fcn),\
};\
static int __init query_init(void){\
   printk(KERN_INFO "Provenance query: loading new query... (%s)\n", name);\
   init_fcn();\
   register_camflow_query_hook(&hooks);\
   return 0;\
}\
static void __exit query_exit(void){\
   printk(KERN_INFO "Provenance query: removing query... (%s)\n", name);\
   unregister_camflow_query_hook(&hooks);\
}\
module_init(query_init);\
module_exit(query_exit);\

#define QUERY_DESCRIPTION(desc) MODULE_DESCRIPTION(desc)
#define QUERY_LICSENSE(license) MODULE_LICENSE(license)
#define QUERY_AUTHOR(author) MODULE_AUTHOR(author)
#define QUERY_VERSION(version) MODULE_VERSION(version)
#define QUERY_NAME(name_str) const char name[] = name_str

static inline int puts(const char *str){
  pr_info("Provenance query: %s\n", str);
  return 0;
}

#define printf(fmt, ...) pr_info(fmt, ##__VA_ARGS__)

static inline bool has_tag(union prov_msg* elmt, tag_t tag){
  return prov_bloom_in(prov_taint(elmt), tag);
}

static inline void add_tag(union prov_msg* elmt, tag_t tag){
  prov_bloom_add(prov_taint(elmt), tag);
}

#else
#error Define a valid target.
#endif
