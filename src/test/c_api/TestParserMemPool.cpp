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


#include <gtest/gtest.h>
#include "../../pbnjson_c/parser_memory_pool.h"

TEST(MemPool, MemPoolOperations)
{
	mem_pool_t mp;
	mempool_init(&mp);

	//free NULL pointer
	mempool_free(&mp, NULL);

	//realloc NULL pointer
	char *p = NULL;
	p = (char*)mempool_realloc(&mp, p, 2);
	EXPECT_TRUE(p);
	strncpy(p, "abc", 3);

	//alloc from pool
	char *p1 = (char*)mempool_malloc(&mp, 2);
	strncpy(p1, "a\0", 2);
	EXPECT_FALSE(strcmp(p, "aba"));

	//realloc last chunk
	p1 = (char*)mempool_realloc(&mp, p1, 4);
	EXPECT_FALSE(strcmp(p1, "a"));

	//realloc inside of pool
	p = (char*)mempool_realloc(&mp, p, MEMORY_POOL_SIZE / 2);
	p[2] = '\0';
	EXPECT_FALSE(strcmp(p, "ab"));

	//realloc from pool to heap
	p = (char*)mempool_realloc(&mp, p, MEMORY_POOL_SIZE + 2);
	EXPECT_FALSE(strcmp(p, "ab"));

	//realloc inside of heap
	p = (char*)mempool_realloc(&mp, p, 2 * MEMORY_POOL_SIZE);
	EXPECT_FALSE(strcmp(p, "ab"));

	//alloc from heap
	char *p2 = (char*)mempool_malloc(&mp, MEMORY_POOL_SIZE);
	EXPECT_TRUE(p2);

	//free from heap
	mempool_free(&mp, p);
	mempool_free(&mp, p2);

	//free from pool
	mempool_free(&mp, p1);
}
