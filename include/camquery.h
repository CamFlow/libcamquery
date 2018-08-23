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
#ifndef ___LIB_CAMQUERY_H
#define ___LIB_CAMQUERY_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/provenance_query.h>
#include <linux/provenance_types.h>

#define QUERY_NAME(name_str) const char name[] = name_str

typedef uint64_t label_t;

#define register_query(init_fcn, hooks)\
static int __init query_init(void){\
   printk(KERN_INFO "Provenance: loading new query... (%s)\n", name);\
   init_fcn();\
   register_provenance_query_hooks(&hooks);\
   return 0;\
}\
static void __exit query_exit(void){\
   printk(KERN_INFO "Provenance: removing query... (%s)\n", name);\
   unregister_provenance_query_hooks(&hooks);\
}\
module_init(query_init);\
module_exit(query_exit);\

#define QUERY_DESCRIPTION(desc) MODULE_DESCRIPTION(desc)
#define QUERY_LICSENSE(license) MODULE_LICENSE(license)
#define QUERY_AUTHOR(author) MODULE_AUTHOR(author)
#define QUERY_VERSION(version) MODULE_VERSION(version)

static inline int puts(const char *str){
  pr_info("Provenance: %s\n", str);
  return 0;
}

#define print(fmt, ...) pr_info(fmt, ##__VA_ARGS__)

#define assign_label(name, str) name = generate_label(str)

static inline bool has_label(prov_entry_t* elmt, label_t label){
  return prov_bloom_in(prov_taint(elmt), label);
}

static inline void add_label(prov_entry_t* elmt, label_t label){
  prov_bloom_add(prov_taint(elmt), label);
}

#endif
