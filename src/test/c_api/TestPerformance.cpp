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
#include <pbnjson.h>
#include <cjson.h>
#include <yajl.h>
#include "PerformanceUtils.hpp"

using namespace std;

namespace {

#if HAVE_CJSON
void ParseCjson(raw_buffer const &input)
{
	unique_ptr<json_object, void(*)(json_object*)>
		o{json_tokener_parse(input.m_str), &json_object_put};
	ASSERT_TRUE(o.get());
	ASSERT_FALSE(is_error(o.get()));
}
#endif // HAVE_CJSON

void ParsePbnjson(raw_buffer const &input, JDOMOptimizationFlags opt, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);
	unique_ptr<jvalue, function<void(jvalue_ref &)>>
		jv{jdom_parse(input, opt, &schemaInfo), [](jvalue_ref &v) { j_release(&v); }};

	ASSERT_FALSE(jis_null(jv.get()));
}

#if HAVE_YAJL
void ParseYajl(raw_buffer const &input)
{
	yajl_callbacks nocb = { 0 };
	unique_ptr<remove_pointer<yajl_handle>::type, void(*)(yajl_handle)>
		handle{
#if YAJL_VERSION < 20000
			yajl_alloc(&nocb, NULL, NULL, NULL),
#else
			yajl_alloc(&nocb, NULL, NULL),
#endif
			&yajl_free
		};

	ASSERT_EQ(yajl_status_ok,
		yajl_parse(handle.get(), (const unsigned char *)input.m_str, input.m_len));
	ASSERT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
	          yajl_parse_complete(handle.get())
#else
	          yajl_complete_parse(handle.get())
#endif
	          );
}
#endif // HAVE_YAJL

void ParseSax(raw_buffer const &input, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	ASSERT_TRUE(jsax_parse(NULL, input, &schemaInfo));
}

const int OPT_NONE = DOMOPT_NOOPT;

const int OPT_ALL = DOMOPT_INPUT_NOCHANGE
                  | DOMOPT_INPUT_OUTLIVES_DOM
                  | DOMOPT_INPUT_NULL_TERMINATED;

} //namespace;

TEST(Performance, ParseSmallInput)
{
	raw_buffer small_inputs[] =
	{
		J_CSTR_TO_BUF("{}"),
		J_CSTR_TO_BUF("[]"),
		J_CSTR_TO_BUF("[\"e1\", \"e2\", \"e3\"]"),
		J_CSTR_TO_BUF("{ \"returnValue\" : true }"),
		J_CSTR_TO_BUF("{ \"returnValue\" : true, \"results\" : [ { \"property\" : \"someName\", \"value\" : 40.5 } ] }")
	};

	cout << "Parsing small JSON (ns):" << endl;
#if HAVE_YAJL
	double ns_yajl = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseYajl(rb);
			}
		});
	cout << "YAJL:\t\t\t" << ns_yajl << endl;
#endif

#if HAVE_CJSON
	double ns_cjson = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseCjson(rb);
			}
		});
	cout << "CJSON:\t\t\t" << ns_cjson << endl;
#endif

	double ns_sax = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseSax(rb, jschema_all());
			}
		});
	cout << "pbnjson-sax:\t\t" << ns_sax << endl;

	double ns_pbnjson = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePbnjson(rb, OPT_NONE, jschema_all());
			}
		});
	cout << "pbnjson (-opts):\t" << ns_pbnjson << endl;

	double ns_pbnjson2 = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePbnjson(rb, OPT_ALL, jschema_all());
			}
		});
	cout << "pbnjson (+opts):\t" << ns_pbnjson2 << endl;

	SUCCEED();
}

TEST(Performance, ParseBigInput)
{
	raw_buffer input = J_CSTR_TO_BUF(
		"{ "
		"\"o1\" : null, "
		"\"o2\" : {}, "
		"\"a1\" : null, "
		"\"a2\" : [], "
		"\"o3\" : {"
			"\"x\" : true, "
			"\"y\" : false, "
			"\"z\" : \"\\\"es'ca'pes'\\\"\""
		"}, "
		"\"n1\" : 0"
		"                              "
		",\"n2\" : 232452312412, "
		"\"n3\" : -233243.653345e-2342 "
		"                              "
		",\"s1\" : \"adfa\","
		"\"s2\" : \"asdflkmsadfl jasdf jasdhf ashdf hasdkf badskjbf a,msdnf ;whqoehnasd kjfbnakjd "
		"bfkjads fkjasdbasdf jbasdfjk basdkjb fjkndsab fjk\","
		"\"a3\" : [ true, false, null, true, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}],"
		"\"a4\" : [[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]],"
		"\"n4\" : 928437987349237893742897234987243987234297982347987249387,"
		"\"b1\" : true"
		"}");

	cout << "Parsing big JSON (ns):" << endl;
#if HAVE_YAJL
	double ns_yajl = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
				ParseYajl(input);
		});
	cout << "YAJL:\t\t\t" << ns_yajl << endl;
#endif

#if HAVE_CJSON
	double ns_cjson = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
				ParseCjson(input);
		});
	cout << "CJSON:\t\t\t" << ns_cjson << endl;
#endif

	double ns_sax = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
				ParseSax(input, jschema_all());
		});
	cout << "pbnjson-sax:\t\t" << ns_sax << endl;

	double ns_pbnjson = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
				ParsePbnjson(input, OPT_NONE, jschema_all());
		});
	cout << "pbnjson (-opts):\t" << ns_pbnjson << endl;

	double ns_pbnjson2 = BenchmarkPerformNs([&](size_t n)
		{
			for (; n > 0; --n)
				ParsePbnjson(input, OPT_ALL, jschema_all());
		});
	cout << "pbnjson (+opts):\t" << ns_pbnjson2 << endl;

	SUCCEED();
}

// vim: set noet ts=4 sw=4:
