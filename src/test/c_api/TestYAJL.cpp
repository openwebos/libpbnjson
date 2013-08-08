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

#include <gtest/gtest.h>
#include <yajl/yajl_version.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_parse.h>
#include <memory>

using namespace std;

struct MyCtxt
{
	int map_open;
	int map_key;
	int map_close;

	int array_open;
	int array_close;

	int string;
	int number;
	int boolean;
	int null;

	bool operator==(MyCtxt const &o) const
	{
		return map_open == o.map_open && map_key == o.map_key && map_close == o.map_close
			&& array_open == o.array_open && array_close == o.array_close
			&& string == o.string && number == o.number && o.boolean == boolean
			&& null == o.null;
	}
};

namespace {

int yos(void *ctxt) { ((MyCtxt *)ctxt)->map_open++; return 1; }
#if YAJL_VERSION < 20000
int yok(void *ctxt, const unsigned char *s, unsigned int len) { ((MyCtxt *)ctxt)->map_key++; return 1; }
#else
int yok(void *ctxt, const unsigned char *s, size_t len) { ((MyCtxt *)ctxt)->map_key++; return 1; }
#endif
int yoc(void *ctxt) { ((MyCtxt *)ctxt)->map_close++; return 1; }

int yas(void *ctxt) { ((MyCtxt *)ctxt)->array_open++; return 1; }
int yac(void *ctxt) { ((MyCtxt *)ctxt)->array_close++; return 1; }

#if YAJL_VERSION < 20000
int yn(void *ctxt, const char *n, unsigned int l) { ((MyCtxt *)ctxt)->number++; return 1; }
int ys(void *ctxt, const unsigned char * s, unsigned int l) { ((MyCtxt *)ctxt)->string++; return 1; }
#else
int yn(void *ctxt, const char *n, size_t l) { ((MyCtxt *)ctxt)->number++; return 1; }
int ys(void *ctxt, const unsigned char * s, size_t l) { ((MyCtxt *)ctxt)->string++; return 1; }
#endif
int yb(void *ctxt, int) { ((MyCtxt *)ctxt)->boolean++; return 1; }
int yN(void *ctxt) { ((MyCtxt *)ctxt)->null++; return 1; }

static yajl_callbacks my_cb =
{
	yN,
	yb,
	NULL,
	NULL,
	yn,
	ys,
	yos,
	yok,
	yoc,
	yas,
	yac,
};

} //namespace

TEST(YAJL, Generator)
{
	unique_ptr<remove_pointer<yajl_gen>::type, void(*)(yajl_gen)>
		g{
#if YAJL_VERSION < 20000
			yajl_gen_alloc(NULL, NULL),
#else
			yajl_gen_alloc(NULL),
#endif
			yajl_gen_free
		};
	yajl_gen_map_open(g.get());
	yajl_gen_map_close(g.get());

	const unsigned char* buf;
#if YAJL_VERSION < 20000
	unsigned int len;
#else
	size_t len;
#endif
	ASSERT_EQ(yajl_gen_get_buf(g.get(), &buf, &len), yajl_gen_status_ok);
	EXPECT_EQ(len, 2);
	EXPECT_STREQ(reinterpret_cast<char const *>(buf), "{}");

	g.reset(
#if YAJL_VERSION < 20000
	        yajl_gen_alloc(NULL, NULL)
#else
	        yajl_gen_alloc(NULL)
#endif
	        );
	yajl_gen_map_open(g.get());
	yajl_gen_string(g.get(), reinterpret_cast<const unsigned char*>("TEST"), sizeof("TEST") - 1);
	yajl_gen_number(g.get(), "53", sizeof("53") - 1);
	yajl_gen_map_close(g.get());
	ASSERT_EQ(yajl_gen_get_buf(g.get(), &buf, &len), yajl_gen_status_ok);
	EXPECT_EQ(len, 11);
	EXPECT_STREQ(reinterpret_cast<const char *>(buf), "{\"TEST\":53}");
}

TEST(YAJL, Parser)
{
	unique_ptr<remove_pointer<yajl_handle>::type, void(*)(yajl_handle)>
		h{nullptr, &yajl_free};

	{
		char const *input = "{}";
		MyCtxt expect = { 1, 0, 1 };

		MyCtxt ctxt = { 0 };
		h.reset(
#if YAJL_VERSION < 20000
		        yajl_alloc(&my_cb, 0, 0, &ctxt)
#else
		        yajl_alloc(&my_cb, 0, &ctxt)
#endif
		        );
		ASSERT_EQ(yajl_status_ok,
			yajl_parse(h.get(), reinterpret_cast<unsigned char const *>(input), strlen(input)));
		EXPECT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
		          yajl_parse_complete(h.get())
#else
		          yajl_complete_parse(h.get())
#endif
		          );
		EXPECT_EQ(expect, ctxt);
	}

	{
		char const *input = "[]";
		MyCtxt expect = { 0, 0, 0, 1, 1, 0, 0, 0, 0 };

		MyCtxt ctxt = { 0 };
		h.reset(
#if YAJL_VERSION < 20000
		        yajl_alloc(&my_cb, 0, 0, &ctxt)
#else
		        yajl_alloc(&my_cb, 0, &ctxt)
#endif
		        );
		ASSERT_EQ(yajl_status_ok,
			yajl_parse(h.get(), reinterpret_cast<unsigned char const *>(input), strlen(input)));
		EXPECT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
		          yajl_parse_complete(h.get())
#else
		          yajl_complete_parse(h.get())
#endif
		          );
		EXPECT_EQ(expect, ctxt);
	}

	{
		char const *input = "{\"returnVal\":0,\"result\":\"foo\",\"param\":true,\"test\":null}";
		MyCtxt expect = { 1, 4, 1, 0, 0, 1, 1, 1, 1 };

		MyCtxt ctxt = { 0 };
		h.reset(
#if YAJL_VERSION < 20000
		        yajl_alloc(&my_cb, 0, 0, &ctxt)
#else
		        yajl_alloc(&my_cb, 0, &ctxt)
#endif
		        );
		ASSERT_EQ(yajl_status_ok,
			yajl_parse(h.get(), reinterpret_cast<unsigned char const *>(input), strlen(input)));
		EXPECT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
		          yajl_parse_complete(h.get())
#else
		          yajl_complete_parse(h.get())
#endif
		          );
		EXPECT_EQ(expect, ctxt);
	}

	{
		char const *input = "[true, false, 5.0, 6114e67,\"adfadsfa\", \"asdfasd\",{},[null, null]]";
		MyCtxt expect = { 1, 0, 1, 2, 2, 2, 2, 2, 2 };

		MyCtxt ctxt = { 0 };
		h.reset(
#if YAJL_VERSION < 20000
		        yajl_alloc(&my_cb, 0, 0, &ctxt)
#else
		        yajl_alloc(&my_cb, 0, &ctxt)
#endif
		        );
		ASSERT_EQ(yajl_status_ok,
			yajl_parse(h.get(), reinterpret_cast<unsigned char const *>(input), strlen(input)));
		EXPECT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
		          yajl_parse_complete(h.get())
#else
		          yajl_complete_parse(h.get())
#endif
		          );
		EXPECT_EQ(expect, ctxt);
	}

	{
		char const *input = "[true, false, {\"a\":[\"b\", \"c\", \"d\", 5, 6, 7,"
			" null, \"see\"], \"b\":{}, \"c\":50.7e90}, null]";
		MyCtxt expect = { 2, 3, 2, 2, 2, 4, 4, 2, 2};

		MyCtxt ctxt = { 0 };
		h.reset(
#if YAJL_VERSION < 20000
		        yajl_alloc(&my_cb, 0, 0, &ctxt)
#else
		        yajl_alloc(&my_cb, 0, &ctxt)
#endif
		        );
		ASSERT_EQ(yajl_status_ok,
			yajl_parse(h.get(), reinterpret_cast<unsigned char const *>(input), strlen(input)));
		EXPECT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
		          yajl_parse_complete(h.get())
#else
		          yajl_complete_parse(h.get())
#endif
		          );
		EXPECT_EQ(expect, ctxt);
	}
}

//vim: set noet ts=4 sw=4:
