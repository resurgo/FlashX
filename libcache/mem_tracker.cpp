/**
 * Copyright 2013 Da Zheng
 *
 * This file is part of SAFSlib.
 *
 * SAFSlib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SAFSlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SAFSlib.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include "mem_tracker.h"
#include "concurrency.h"
#include "debugger.h"

atomic_number<size_t> alloc_objs;
atomic_number<size_t> alloc_bytes;

#ifdef ENABLE_MEM_TRACE

class print_mem_tracker_task: public debug_task
{
public:
	void run() {
		printf("alloc %ld objs and %ld bytes\n", alloc_objs.get(),
				alloc_bytes.get());
	}
};

void init_mem_tracker()
{
	printf("register mem_tracker to debug\n");
	debug.register_task(new print_mem_tracker_task());
}

void *operator new(size_t n) throw (std::bad_alloc)
{
	alloc_objs.inc(1);
	alloc_bytes.inc(n);
	void *p = malloc(n + sizeof(size_t));
	size_t *size = (size_t *) p;
	*size = n;

	return (void *) (size + 1);
}

void operator delete(void *p) throw ()
{
	if (p == NULL)
		return;

	alloc_objs.dec(1);
	size_t *size = (size_t *) (((char *) p) - sizeof(size_t));
	alloc_bytes.dec(*size);
	free(size);
}

void *operator new[](size_t n) throw (std::bad_alloc)
{
	alloc_objs.inc(1);
	alloc_bytes.inc(n);
	void *p = malloc(n + sizeof(size_t));
	size_t *size = (size_t *) p;
	*size = n;

	return (void *) (size + 1);
}

void operator delete[](void *p) throw ()
{
	if (p == NULL)
		return;

	alloc_objs.dec(1);
	size_t *size = (size_t *) (((char *) p) - sizeof(size_t));
	alloc_bytes.dec(*size);
	free(size);
}

#endif

size_t get_alloc_objs()
{
	return alloc_objs.get();
}

size_t get_alloc_bytes()
{
	return alloc_bytes.get();
}
