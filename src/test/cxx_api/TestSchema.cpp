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
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

TEST(TestSchema, OneOf)
{
	auto schema = JSchemaFragment("{\"oneOf\":["
		"{\"additionalProperties\":{\"type\":\"string\"}},"
		"{\"additionalProperties\":{\"type\":\"integer\"}}"
	"]}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

TEST(TestSchema, LocalRef)
{
	auto schema = JSchemaFragment("{"
		"\"definitions\":{"
			"\"foo\":{\"additionalProperties\":{\"type\":\"string\"}},"
			"\"bar\":{\"additionalProperties\":{\"type\":\"integer\"}}"
		"},"
		"\"oneOf\":["
			"{\"$ref\":\"#/definitions/foo\"},"
			"{\"$ref\":\"#/definitions/bar\"}"
		"]"
	"}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

TEST(TestSchema, LocalRefEscaped)
{
	auto schema = JSchemaFragment("{"
		"\"definitions\":{"
			"\"foo~1\":{\"additionalProperties\":{\"type\":\"string\"}},"
			"\"bar/schema\":{\"additionalProperties\":{\"type\":\"integer\"}}"
		"},"
		"\"oneOf\":["
			"{\"$ref\":\"#/definitions/foo~01\"},"
			"{\"$ref\":\"#/definitions/bar~1schema\"}"
		"]"
	"}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

TEST(TestSchema, LocalRefRef)
{
	auto schema = JSchemaFragment("{"
		"\"definitions\":{"
			"\"foo\":{\"additionalProperties\":{\"type\":\"string\"}},"
			"\"bar\":{\"additionalProperties\":{\"type\":\"integer\"}},"
			"\"foofoo\":{\"$ref\":\"#/definitions/foo\"}"
		"},"
		"\"oneOf\":["
			"{\"$ref\":\"#/definitions/foofoo\"},"
			"{\"$ref\":\"#/definitions/bar\"}"
		"]"
	"}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

// ex: set noet ts=4 sw=4 tw=80:
