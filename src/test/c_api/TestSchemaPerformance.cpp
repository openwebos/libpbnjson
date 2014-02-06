// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
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
#include "PerformanceUtils.hpp"

using namespace std;

namespace {

void ParsePbnjson(raw_buffer const &input, JDOMOptimizationFlags opt, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	ASSERT_TRUE(jsax_parse(NULL, input, &schemaInfo));
}

void BenchmarkSchemas(raw_buffer input, vector<string>& schema_jsons)
{
	cout << "Parsing JSON (MBps): " << endl << string(input.m_str, input.m_len) << endl << endl;
	for (auto const &sj : schema_jsons)
	{
		raw_buffer schema_str;
		schema_str.m_str = sj.c_str();
		schema_str.m_len = sj.length();
		unique_ptr<jschema, function<void(jschema_ref &)>> schema
			{
				sj.empty() ? jschema_all() : jschema_parse(schema_str, JSCHEMA_DOM_NOOPT, NULL),
				[](jschema_ref &s) { if(s != jschema_all()) jschema_release(&s); }
			};

		ASSERT_TRUE(schema.get());

		cout << "with schema: " << (sj.empty() ? "schema_all()" : sj) << endl;
		double s_pbnjson = BenchmarkPerform([&](size_t n)
			{
				for (; n > 0; --n)
					ParsePbnjson(input, DOMOPT_NOOPT, schema.get());
			});
		cout << ConvertToMBps(schema_str.m_len, s_pbnjson) << endl << endl;
	}
}
} //namespace;

TEST(SchemaPerformance, StringTypeSchema)
{
	raw_buffer input = J_CSTR_TO_BUF("\"test string\"");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"string\"}",
		"{\"type\":\"string\", \"maxLength\":15 }",
		"{\"type\":\"string\", \"maxLength\":15, \"minLength\":10 }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, NumberTypeSchema)
{
	raw_buffer input = J_CSTR_TO_BUF("1.23");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"number\"}",
		"{\"type\":\"number\", \"maximum\":2 }",
		"{\"type\":\"number\", \"maximum\":2, \"minimum\":1 }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, IntegerTypeSchema)
{
	raw_buffer input = J_CSTR_TO_BUF("123");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"integer\"}",
		"{\"type\":\"integer\", \"maximum\":200 }",
		"{\"type\":\"integer\", \"maximum\":200, \"minimum\":1 }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, BooleanTypeSchema)
{
	raw_buffer input = J_CSTR_TO_BUF("true");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"boolean\"}",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, ArrayTypeSchemaEmpty)
{
	raw_buffer input = J_CSTR_TO_BUF("[]");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"array\"}",
		"{\"type\":\"array\", \"items\":{} }",
		"{\"type\":\"array\", \"items\":[] }",
		"{\"type\":\"array\", \"items\":[], \"additionalItems\":false }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, ArrayTypeSchemaOneType)
{
	raw_buffer input = J_CSTR_TO_BUF("[\"a\", \"b\", \"c\"]");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"array\"}",
		"{\"type\":\"array\", \"items\":{} }",
		"{\"type\":\"array\", \"items\":[] }",
		"{\"type\":\"array\", \"items\":[], \"additionalItems\":{\"type\": \"string\"} }",
		"{\"type\":\"array\", \"items\":{\"type\": \"string\"} }",
		"{\"type\":\"array\", \"items\":[{\"type\": \"string\"}] }",
		"{\"type\":\"array\", \"items\":[{\"type\": \"string\"}, {\"type\": \"string\"}, {\"type\": \"string\"}] }",
		"{\"type\":\"array\", \"items\":[{\"type\": \"string\"}, {\"type\": \"string\"}, {\"type\": \"string\"}], \"additionalItems\":false }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, ArrayTypeSchemaMultipleTypes)
{
	raw_buffer input = J_CSTR_TO_BUF("[\"a\", 1, null, [], {}]");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"type\":\"array\"}",
		"{\"type\":\"array\", \"items\":{} }",
		"{\"type\":\"array\", \"items\":[] }",
		"{\"type\":\"array\", \"items\":[{\"type\": \"string\"}] }",
		"{\"type\":\"array\", \"items\":[{\"type\": \"string\"}, {}, {}, {}] }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, ObjectTypeSchema)
{
	raw_buffer input = J_CSTR_TO_BUF("{\"a\": null, \"b\": \"hello\", \"c\":[]}");
	vector<string> schema_jsons =
	{
		"",

		"{}",

		"{\"type\":\"object\"}",

		"{"
			"\"type\":\"object\", "
			"\"properties\":{"
				"\"a\":{} "
			"} "
		"}",

		"{"
			"\"type\":\"object\", "
			"\"properties\":{"
				"\"a\":{}, "
				"\"b\":{}, "
				"\"c\":{} "
			"} "
		"}",

		"{"
			"\"type\":\"object\", "
			"\"properties\":{"
				"\"a\":{}, "
				"\"b\":{}, "
				"\"c\":{} "
			"}, "
			"\"required\": [\"a\"] "
		"}",

		"{"
			"\"type\":\"object\", "
			"\"properties\":{"
				"\"a\":{}, "
				"\"b\":{}, "
				"\"c\":{} "
			"}, "
			"\"required\": [\"a\", \"b\", \"c\"] "
		"}",

		"{"
			"\"type\":\"object\", "
			"\"required\": [\"a\"] "
		"}",

		"{"
			"\"type\":\"object\", "
			"\"required\": [\"a\", \"b\", \"c\"] "
		"}",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, EnumSchemaOneNumber)
{
	raw_buffer input = J_CSTR_TO_BUF("12");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"enum\":[12]}",
		"{\"enum\":[12, null] }",
		"{\"enum\":[12, null, \"abc\", {\"a\": 12}] }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, EnumSchemaOneObject)
{
	raw_buffer input = J_CSTR_TO_BUF("{\"a\": null, \"b\": [1, 2, 3] } ");
	vector<string> schema_jsons =
	{
		"",
		"{}",
		"{\"enum\":[{\"a\": null, \"b\": [1, 2, 3]}] }",
		"{\"enum\":[null, {\"a\": \"abc\"}, {\"a\": null, \"b\": [1, 2, 3]}] }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, EnumSchemaMultiple)
{
	raw_buffer input = J_CSTR_TO_BUF("[null, 12, \"abc\", [1, 2, 3], {\"a\": true}]");
	vector<string> schema_jsons =
	{
		"",
		"{\"type\":\"array\", \"items\":{} }",
		"{\"type\":\"array\", \"items\":{\"enum\":[null, 12, \"abc\", [1, 2, 3], {\"a\": true}] } }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, AllOfSchema)
{
	raw_buffer input = J_CSTR_TO_BUF("{\"a\": true, \"b\": [], \"c\": 12}");
	vector<string> schema_jsons =
	{
		"",

		"{}",

		"{\"allOf\":[{}] }",

		"{\"allOf\":["
			"{\"type\": \"object\"}, "
			"{}"
		"] }",

		"{\"allOf\":["
			"{\"type\": \"object\"}, "
			"{}"
		"] }",

		"{\"allOf\":["
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {\"type\": \"boolean\"}"
				"}"
			"}, "
			"{}"
		"] }",

		"{\"allOf\":["
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {\"type\": \"boolean\"}"
				"}"
			"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{}"
		"] }",

		"{\"allOf\":["
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {\"type\": \"boolean\"}"
				"}"
			"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"c\": {\"type\": \"number\"}"
				"}"
			"}"
		"] }",

		"{\"allOf\":["
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {\"type\": \"boolean\"}"
				"}"
			"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"c\": {\"type\": \"number\"}"
				"}"
			"}, "
			"{}"
		"] }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, AnyOfSchemaOneItem)
{
	raw_buffer input = J_CSTR_TO_BUF("null");
	vector<string> schema_jsons =
	{
		"",

		"{}",

		"{\"anyOf\":[{}] }",

		"{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{}"
		"] }",

		"{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}"
		"] }",

		"{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{}"
		"] }",

		"{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{\"type\": \"object\"}, "
			"{}"
		"] }",

		"{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{}"
		"] }",

		"{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{\"enum\": [123, [1, 2, 3], [\"a\", \"b\", \"c\"], {\"test\":\"test\"}, null]}, "
			"{}"
		"] }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, AnyOfSchemaMultipleItems)
{
	raw_buffer input = J_CSTR_TO_BUF("[null, 12]");
	vector<string> schema_jsons =
	{
		"",

		"{\"type\":\"array\", \"items\":{} }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":[{}] } }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{\"type\": \"object\"}, "
			"{}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"anyOf\":["
			"{\"type\": \"null\"}, "
			"{\"type\": \"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{\"enum\": [123, [1, 2, 3], [\"a\", \"b\", \"c\"], {\"test\":\"test\"}, null]}, "
			"{}"
		"] } }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, OneOfSchemaOneItmem)
{
	raw_buffer input = J_CSTR_TO_BUF("null");
	vector<string> schema_jsons =
	{
		"",

		"{}",

		"{\"oneOf\":[{}] }",

		"{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}"
		"] }",

		"{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}, "
			"{\"type\":\"object\"}"
		"] }",

		"{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}"
		"] }",

		"{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{\"enum\": [123, [1, 2, 3], [\"a\", \"b\", \"c\"], {\"test\":\"test\"}]}"
		"] }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, OneOfSchemaMultipleItems)
{
	raw_buffer input = J_CSTR_TO_BUF("[null, 12]");
	vector<string> schema_jsons =
	{
		"",

		"{\"type\":\"array\", \"items\":{} }",

		"{\"type\":\"array\", \"items\":{\"oneOf\":[{}] } }",

		"{\"type\":\"array\", \"items\":{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}, "
			"{\"type\":\"object\"}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}"
		"] } }",

		"{\"type\":\"array\", \"items\":{\"oneOf\":["
			"{\"type\":\"null\"}, "
			"{\"type\":\"number\"}, "
			"{"
				"\"type\": \"object\", "
				"\"properties\": {"
					"\"a\": {}, "
					"\"id\": {\"type\": \"integer\"}, "
					"\"b\": {\"type\": \"array\"}"
				"}"
			"}, "
			"{\"enum\": [123, [1, 2, 3], [\"a\", \"b\", \"c\"], {\"test\":\"test\"}]}"
		"] } }",
	};

	BenchmarkSchemas(input, schema_jsons);
}

TEST(SchemaPerformance, WideRangeSchemas)
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

	vector<string> schema_jsons =
	{
		"",

		"{}",

		"{ \"type\" : \"object\" }",

		"{"
			"\"type\" : \"object\", "
			"\"properties\" : {"
				"\"o1\" : {"
					"\"type\" : \"null\""
				"}"
			"}"
		"}",

		"{"
			"\"type\" : \"object\", "
			"\"properties\" : {"
				"\"o1\" : {"
					"\"type\" : \"null\""
				"}"
			"}, "
			"\"additionalProperties\" : {}"
		"}",

		"{"
			"\"type\" : \"object\", "
			"\"properties\" : {"
				"\"o1\" : {"
					"\"type\" : \"null\""
				"}, "
				"\"o2\" : {"
					"\"type\" : \"object\", "
					"\"additionalProperties\" : false"
				"}, "
				"\"a1\" : {"
					"\"type\" : \"null\""
				"}, "
				"\"a2\" : {"
					"\"type\" : [\"array\", \"null\"], "
					"\"maxItems\" : 0"
				"}, "
				"\"o3\" : {"
					"\"type\" : \"object\", "
					"\"additionalProperties\" : {"
						"\"type\" : [\"boolean\", \"string\"]"
					"}"
				"}, "
				"\"n1\" : {"
					"\"type\" : \"integer\""
				"}, "
				"\"n2\" : {"
					"\"type\" : \"integer\""
				"}, "
				"\"n3\" : {"
					"\"type\" : \"number\""
				"}, "
				"\"s1\" : {"
					"\"type\" : \"string\", "
					"\"maxLength\" : 5"
				"}, "
				"\"s2\" : {"
					"\"type\" : \"string\""
				"}, "
				"\"a3\" : {"
					"\"type\" : \"array\", "
					"\"items\" : {"
						"\"type\" : [\"boolean\", \"null\", \"object\"]"
					"}"
				"}, "
				"\"a4\" : {"
					"\"type\" : \"array\", "
					"\"items\" : {"
						"\"type\" : \"array\""
					"}"
				"} "
			"}, "
			"\"additionalProperties\" : {}"
		"}",
	};

	BenchmarkSchemas(input, schema_jsons);

	SUCCEED();
}
