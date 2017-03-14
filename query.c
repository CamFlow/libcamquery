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

#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
#include <linux/camflow_query.h>

#define QUERY_NAME "Example Query"

MODULE_LICENSE("GPL");              ///< The license type -- this affects runtime behavior
MODULE_AUTHOR("Thomas Pasquier");      ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION(QUERY_NAME);  ///< The description -- see modinfo
MODULE_VERSION("0.1");              ///< The version of the module

static char *name = "world";        ///< An example LKM argument -- default value is "world"
module_param(name, charp, S_IRUGO); ///< Param desc. charp = char ptr, S_IRUGO can be read/not changed
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");  ///< parameter description

static int out_edge(union prov_msg* node, union prov_msg* edge){
  printk(KERN_INFO QUERY_NAME" out edge.\n");
  return 0;
}

static int in_edge(union prov_msg* edge, union prov_msg* node){
  printk(KERN_INFO QUERY_NAME" in edge.\n");
  return 0;
}

struct provenance_query_hooks hooks = {
  QUERY_HOOK_INIT(out_edge, out_edge),
  QUERY_HOOK_INIT(in_edge, in_edge),
};

static int __init query_init(void){
   printk(KERN_INFO "CamFlow: loading new query... (" QUERY_NAME ")\n");
   register_camflow_query_hook(&hooks);
   return 0;
}

static void __exit query_exit(void){
   printk(KERN_INFO "CamFlow: removing query... (" QUERY_NAME ")\n");
   unregister_camflow_query_hook(&hooks);
}

module_init(query_init);
module_exit(query_exit);
