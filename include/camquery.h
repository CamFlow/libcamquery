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
#ifndef ___LIB_CAMQUERY_H
#define ___LIB_CAMQUERY_H


#define QUERY_NAME(name_str) const char name[] = name_str

#ifdef KERNEL_QUERY
#include "kernel_query.h"
#else
#include "service_query.h"
#endif
#endif
