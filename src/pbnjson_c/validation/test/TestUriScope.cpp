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

#include "../uri_scope.h"
#include <gtest/gtest.h>

TEST(TestUriScope, UriStack)
{
	UriScope *u = uri_scope_new();
	ASSERT_TRUE(u != NULL);

	char buffer[1024];

	uri_scope_push_uri(u, "http://a.b.c/test.json#/definitions/x");
	{
		int chars_required = uri_scope_get_document_length(u);
		EXPECT_STREQ("http://a.b.c/test.json",
		             uri_scope_get_document(u, buffer, chars_required));
		EXPECT_STREQ("#/definitions/x", uri_scope_get_fragment(u));
	}
	uri_scope_push_uri(u, "a/qwer.json#a");
	{
		int chars_required = uri_scope_get_document_length(u);
		EXPECT_STREQ("http://a.b.c/a/qwer.json",
		             uri_scope_get_document(u, buffer, chars_required));
		EXPECT_STREQ("#a", uri_scope_get_fragment(u));
	}
	uri_scope_push_uri(u, "file:///another.json");
	{
		int chars_required = uri_scope_get_document_length(u);
		EXPECT_STREQ("file:///another.json",
		             uri_scope_get_document(u, buffer, chars_required));
		EXPECT_STREQ("#", uri_scope_get_fragment(u));
	}


	uri_scope_pop_uri(u);
	EXPECT_STREQ("http://a.b.c/a/qwer.json",
	             uri_scope_get_document(u, buffer, 1024));
	EXPECT_STREQ("#a", uri_scope_get_fragment(u));

	uri_scope_pop_uri(u);
	EXPECT_STREQ("http://a.b.c/test.json",
	             uri_scope_get_document(u, buffer, 1024));
	EXPECT_STREQ("#/definitions/x", uri_scope_get_fragment(u));

	uri_scope_pop_uri(u);
	uri_scope_free(u);
}

TEST(TestUriScope, EmptyBase)
{
	UriScope *u = uri_scope_new();
	ASSERT_TRUE(u != NULL);

	char buffer[1024];

	uri_scope_push_uri(u, "");
	uri_scope_push_uri(u, "path/to/base.json#a");
	{
		int chars_required = uri_scope_get_document_length(u);
		EXPECT_STREQ("path/to/base.json",
		             uri_scope_get_document(u, buffer, chars_required));
		EXPECT_STREQ("#a", uri_scope_get_fragment(u));
	}

	uri_scope_free(u);
}

TEST(TestUriScope, EscapeJsonPointer)
{
	char buffer[64];
	EXPECT_STREQ("", escape_json_pointer("", buffer));
	EXPECT_STREQ("a~1b", escape_json_pointer("a/b", buffer));
	EXPECT_STREQ("c%%d", escape_json_pointer("c%%d", buffer));
	EXPECT_STREQ("e^f", escape_json_pointer("e^f", buffer));
	EXPECT_STREQ("g|h", escape_json_pointer("g|h", buffer));
	EXPECT_STREQ("i\\j", escape_json_pointer("i\\j", buffer));
	EXPECT_STREQ("k\"l", escape_json_pointer("k\"l", buffer));
	EXPECT_STREQ(" ", escape_json_pointer(" ", buffer));
	EXPECT_STREQ("m~0n", escape_json_pointer("m~n", buffer));
}

TEST(TestUriScope, UnescapeJsonPointer)
{
	char buffer[64];
	EXPECT_STREQ("", unescape_json_pointer("", buffer));
	EXPECT_STREQ("/", unescape_json_pointer("/", buffer));
	EXPECT_STREQ("a/b", unescape_json_pointer("a~1b", buffer));
	EXPECT_STREQ("/c%%d", unescape_json_pointer("/c%%d", buffer));
	EXPECT_STREQ("/e^f", unescape_json_pointer("/e^f", buffer));
	EXPECT_STREQ("/g|h", unescape_json_pointer("/g|h", buffer));
	EXPECT_STREQ("/i\\j", unescape_json_pointer("/i\\j", buffer));
	EXPECT_STREQ("/k\"l", unescape_json_pointer("/k\"l", buffer));
	EXPECT_STREQ("/ ", unescape_json_pointer("/ ", buffer));
	EXPECT_STREQ("/m~n", unescape_json_pointer("/m~0n", buffer));
}

TEST(TestUriScope, FragmentLeaf)
{
	UriScope *u = uri_scope_new();
	ASSERT_TRUE(u != NULL);

	uri_scope_push_uri(u, "#");
	EXPECT_STREQ("#/definitions", uri_scope_push_fragment_leaf(u, "definitions"));
	EXPECT_STREQ("#/definitions/a", uri_scope_push_fragment_leaf(u, "a"));

	EXPECT_STREQ("#/definitions", uri_scope_pop_fragment_leaf(u));
	EXPECT_STREQ("#", uri_scope_pop_fragment_leaf(u));

	uri_scope_pop_uri(u);
	uri_scope_free(u);
}
