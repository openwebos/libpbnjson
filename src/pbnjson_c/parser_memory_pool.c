// @@@LICENSE
//
//      Copyright (c) 2014 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#include <stdlib.h>
#include <string.h>

#include "parser_memory_pool.h"


void mempool_init(mem_pool_t *m)
{
	m->end = m->begin + sizeof(m->begin);
	m->prev = m->begin;
	m->current = m->begin;
}

void mempool_free(void *ctx, void *p)
{
	mem_pool_t *m = (mem_pool_t*)ctx;
	if (p && ((char*)p < m->begin || p >= m->end))
		free(p);
}

void* mempool_malloc(void *ctx, yajl_size_t size)
{
	mem_pool_t *m = (mem_pool_t*)ctx;

	if ((char*)m->end >= (char*)m->current + size) {
		m->prev = m->current;
		m->current = (char*)m->prev + size;
		return m->prev;
	}
	return malloc(size); // can not allocate from pool
}

void* mempool_realloc(void *ctx, void *p, yajl_size_t size)
{
	mem_pool_t *m = (mem_pool_t*)ctx;

	if (p && ((char*)p < m->begin || p >= m->end)) // p from heap
		return realloc(p, size);

	if (p == m->prev && (char*)m->end >= (char*)p + size) { // p last chunk in pool
		m->current = (char*)p + size;
		return p;
	}
	// p inside pool or null pointer
	char *top = m->current;
	void *newp = mempool_malloc(ctx, size);
	if (p) {
		size_t diff = top - (char*)p;
		size_t sz = (diff < size) ? diff : size;
		memcpy(newp, p, sz);
	}
	return newp;
}
