// @@@LICENSE
//
//      Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include "../uri_resolver.h"
#include <gtest/gtest.h>

TEST(TestUriResolver, AddDocument)
{
	UriResolver *u = uri_resolver_new();
	ASSERT_TRUE(u != NULL);

	EXPECT_STREQ("a.b.c", uri_resolver_add_document(u, "a.b.c"));
	EXPECT_STREQ("c.d.f", uri_resolver_add_document(u, "c.d.f"));

	char *dump = uri_resolver_dump(u);
	EXPECT_STREQ("a.b.c:\nc.d.f:\n", dump);
	g_free(dump);

	uri_resolver_free(u);
}
