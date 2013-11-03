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

#include "memory_manager.h"

const long SHRINK_NPAGES = 1024;
const long INCREASE_SIZE = 1024 * 1024 * 128;

memory_manager::memory_manager(
		long max_size, int node_id): slab_allocator(
			std::string("mem_manager-") + itoa(node_id), PAGE_SIZE,
			INCREASE_SIZE <= max_size ? INCREASE_SIZE : max_size,
			// We don't initialize pages but we pin pages.
			max_size, node_id, false, true) {
}

/**
 * get `npages' pages for `request_cache'.
 * In the case of shrinking caches, it makes no sense
 * to shrink the cache that is requesting free pages.
 */
bool memory_manager::get_free_pages(int npages,
		char **pages, page_cache *request_cache) {
	int ret = slab_allocator::alloc(pages, npages);
	/* 
	 * slab_allocator allocates either all required number of 
	 * pages or none of them.
	 */
	if (ret == 0) {
		long size = 0;
		page_cache *cache = NULL;
		/* shrink the cache of the largest size */
		for (unsigned int i = 0; i < caches.size(); i++) {
			if (size < caches[i]->size()) {
				size = caches[i]->size();
				cache = caches[i];
			}
		}
		if (cache == NULL)
			return false;
		/* 
		 * if we are going to shrink the cache that requests
		 * free pages, it just fails.
		 */
		if (request_cache == cache) {
			return false;
		}
		int num_shrink = SHRINK_NPAGES;
		if (num_shrink < npages)
			num_shrink = npages;
		char *buf[num_shrink];
		if (!cache->shrink(num_shrink, buf)) {
			return false;
		}
		slab_allocator::free(buf, num_shrink);
		/* now it's guaranteed that we have enough free pages. */
		ret = slab_allocator::alloc(pages, npages);
	}
	return true;
}

void memory_manager::free_pages(int npages, char **pages) {
	slab_allocator::free(pages, npages);
}
