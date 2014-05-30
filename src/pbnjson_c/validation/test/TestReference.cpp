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

#include "../parser_api.h"
#include "../uri_resolver.h"
#include "../validator.h"
#include "../validation_api.h"
#include <gtest/gtest.h>


TEST(TestReference, First)
{
	auto u = uri_resolver_new();
	ASSERT_TRUE(u != NULL);
	auto v = parse_schema(
		"{"
			"\"id\": \"file://test.json\","
			"\"definitions\": {"
				"\"b\": {\"type\": \"boolean\"}"
			"},"
			"\"type\": \"object\","
			"\"properties\": {\"a\": {\"$ref\": \"#/definitions/b\"}}"
		"}",
		u, "file://test2.json",
		NULL, NULL
		);
	ASSERT_TRUE(v != NULL);

	EXPECT_TRUE(validate_json("{\"a\": true}", v, u, NULL));
	EXPECT_FALSE(validate_json("{\"a\": \"b\"}", v, u, NULL));
	EXPECT_FALSE(validate_json("{\"a\": null}", v, u, NULL));

	validator_unref(v);
	uri_resolver_free(u);
}

TEST(TestReference, selfReference)
{
	auto u = uri_resolver_new();
	ASSERT_TRUE(u != NULL);
	auto v = parse_schema( R"schema(
		{
			"type": "object",
			"properties": {
				"name"  : {"type": "string" },
				"self"  : {"$ref": "#"}
			}
		})schema",
		u, "file:///test3.json",
		NULL, NULL
		);
	ASSERT_TRUE(v != NULL);

	EXPECT_TRUE(validate_json(R"({"name": "b"})", v, u, NULL));
	EXPECT_FALSE(validate_json("{\"name\": true}", v, u, NULL));
	EXPECT_TRUE(validate_json(R"({"self" : {"name": "b"}})", v, u, NULL));
	EXPECT_FALSE(validate_json(R"({"self" : {"name": null}})", v, u, NULL));

	validator_unref(v);
	uri_resolver_free(u);
}
