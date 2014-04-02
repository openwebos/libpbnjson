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

#ifndef PARSER_MEMORY_POOL_H_
#define PARSER_MEMORY_POOL_H_

#include "yajl_compat.h"


//5120 is enough to allocate 4096 buffer and other yajl structures
#define MEMORY_POOL_SIZE 5120

/**
 * Memory pool type for YAJL parser
 *
 * 0                                 MEMORY_POOL_SIZE
 * xxxxxxxxxxxxxxxXXXXXX-------------------->|
 * |              |     |                    |
 * |  allocated   |last |      free          |
 * |              |     |                    |
 * +              +     +                    +
 * begin        prev   current              end
 *
 */
typedef struct memory_pool {
	char begin[MEMORY_POOL_SIZE];
	void *end;     ///< End of the pool
	void *prev;    ///< Pointer to the last allocated chunk
	void *current; ///< Pointer to the next free memory
} mem_pool_t;

#ifdef __cplusplus
extern "C" {
#endif

void mempool_init(mem_pool_t *m);

void mempool_free(void *ctx, void *p);

void* mempool_malloc(void *ctx, yajl_size_t size);

void* mempool_realloc(void *ctx, void *p, yajl_size_t size);

#ifdef __cplusplus
}
#endif

#endif /* PARSER_MEMORY_POOL_H_ */
