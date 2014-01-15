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
#include "../validation_api.h"
#include <gtest/gtest.h>

using namespace std;

class Json : public ::testing::Test
{
protected:
	Validator *v;

	virtual void SetUp()
	{
		v = NULL;
	}

	virtual void TearDown()
	{
		validator_unref(v), v = NULL;
	}
};

TEST_F(Json, First)
{
	v = parse_schema("{}", 0, 0, 0, 0);
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("{\"success\": true}", v));
}

TEST_F(Json, Null)
{
	v = parse_schema_bare("{\"type\": \"null\"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	ValidationError error;
	EXPECT_FALSE(validate_json("false", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NULL, error.error);
	EXPECT_FALSE(validate_json("{}", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NULL, error.error);
	EXPECT_FALSE(validate_json("\"hello\"", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NULL, error.error);
}

TEST_F(Json, Boolean)
{
	v = parse_schema_bare("{\"type\": \"boolean\"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("\"asdf\"", v));
}

TEST_F(Json, Number)
{
	v = parse_schema_bare("{\"type\": \"number\"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("-1.2", v));
	EXPECT_TRUE(validate_json_plain("-1.2e2", v));
	ValidationError error;
	EXPECT_FALSE(validate_json("false", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NUMBER, error.error);
	EXPECT_FALSE(validate_json("{}", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NUMBER, error.error);
	EXPECT_FALSE(validate_json("\"hello\"", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NUMBER, error.error);
	EXPECT_FALSE(validate_json("[]", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NUMBER, error.error);
	EXPECT_FALSE(validate_json("null", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NUMBER, error.error);
}

TEST_F(Json, Integer)
{
	v = parse_schema_bare("{\"type\": [\"integer\"] }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
}

TEST_F(Json, IntegerWithRestrictions)
{
	v = parse_schema_bare("{\"type\": \"integer\", \"maximum\": 10, \"minimum\": 2 }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("3", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("11", v));
	EXPECT_TRUE(validate_json_plain("9", v));
}

TEST_F(Json, CombinedTypesOnlyNull)
{
	v = parse_schema_bare("{\"type\": [\"null\"] }");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, CombinedTypesStringAndInteger)
{
	v = parse_schema_bare("{\"type\": [\"string\", \"integer\"] }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, CombinedTypesStringAndIntegerWithRestrictions)
{
	v = parse_schema_bare("{\"type\": [\"string\", \"integer\"], \"maximum\": 10, \"maxLength\": 3 }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("11", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("\"hello\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, CombinedTypesAllTypes)
{
	v = parse_schema_bare("{\"type\": [\"null\", \"boolean\", \"number\", \"string\", \"object\", \"array\"] }");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
}

TEST_F(Json, CombinedTypesNumberAndInteger)
{
	v = parse_schema_bare("{\"type\": [\"number\", \"integer\"] }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, CombinedTypesIntegerAndNumber)
{
	v = parse_schema_bare("{\"type\": [\"integer\", \"number\"] }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, NoTypesDefinedWithMaximum)
{
	v = parse_schema_bare("{\"maximum\":10}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
	EXPECT_FALSE(validate_json_plain("11", v));
}

TEST_F(Json, NoTypesDefinedWithMaxLen)
{
	v = parse_schema_bare("{\"maxLength\":10}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
	EXPECT_FALSE(validate_json_plain("\"0123456789!!!\"", v));
}

TEST_F(Json, NoTypesDefinedWithItems)
{
	v = parse_schema_bare("{\"items\":{\"type\":\"null\"} }");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\"]", v));
}

TEST_F(Json, NoTypesDefinedWithProperties)
{
	v = parse_schema_bare(
		"{"
			"\"properties\": {"
				"\"a\": {"
					"\"type\":\"null\""
				"}"
			"}"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("1.2", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":true}", v));
}

TEST_F(Json, NumberMaximum)
{
	v = parse_schema_bare(
		"{\"type\": \"number\","
			"\"maximum\": 1.23e2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("123", v));
	EXPECT_FALSE(validate_json_plain("124", v));
	EXPECT_FALSE(validate_json_plain("123.0001", v));
	EXPECT_TRUE(validate_json_plain("-123.0001", v));
}

TEST_F(Json, NumberMinimum)
{
	v = parse_schema_bare(
		"{\"type\": \"number\","
			"\"minimum\": -1.23e2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("123", v));
	EXPECT_TRUE(validate_json_plain("-123", v));
	EXPECT_FALSE(validate_json_plain("-124", v));
	EXPECT_FALSE(validate_json_plain("-123.0001", v));
	EXPECT_TRUE(validate_json_plain("123.0001", v));
}

TEST_F(Json, NumberExclusiveMaximum)
{
	v = parse_schema_bare(
		"{\"type\": \"number\","
			"\"maximum\": 1.23e2,"
			"\"exclusiveMaximum\": true"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("122.999", v));
	EXPECT_FALSE(validate_json_plain("123", v));
	EXPECT_FALSE(validate_json_plain("123.0001", v));
}

TEST_F(Json, NumberExclusiveMinimum)
{
	v = parse_schema_bare(
		"{\"type\": \"number\","
			"\"minimum\": -1.23e2,"
			"\"exclusiveMinimum\": true"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("-122.999", v));
	EXPECT_FALSE(validate_json_plain("-123", v));
	EXPECT_FALSE(validate_json_plain("-123.0001", v));
}

TEST_F(Json, NumberMultipleOfInteger)
{
	v = parse_schema_bare(
		"{\"type\": \"number\","
			"\"multipleOf\": 2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("2", v));
	EXPECT_FALSE(validate_json_plain("2.1", v));
	EXPECT_TRUE(validate_json_plain("4", v));
	EXPECT_FALSE(validate_json_plain("5", v));
	EXPECT_TRUE(validate_json_plain("-2", v));
	EXPECT_TRUE(validate_json_plain("-4", v));
	EXPECT_FALSE(validate_json_plain("-5", v));
}

TEST_F(Json, NumberMultipleOf)
{
	v = parse_schema_bare(
		"{\"type\": \"number\","
			"\"multipleOf\": 2.5"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("2.5", v));
	EXPECT_FALSE(validate_json_plain("4", v));
	EXPECT_TRUE(validate_json_plain("5", v));
	EXPECT_FALSE(validate_json_plain("5.1", v));
	EXPECT_TRUE(validate_json_plain("-2.5", v));
	EXPECT_FALSE(validate_json_plain("-4", v));
	EXPECT_TRUE(validate_json_plain("-5", v));
}

TEST_F(Json, IntegerMultipleOf)
{
	v = parse_schema_bare(
		"{\"type\": \"integer\","
			"\"multipleOf\": 2.5"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("2.5", v));
	EXPECT_FALSE(validate_json_plain("4", v));
	EXPECT_TRUE(validate_json_plain("5", v));
	EXPECT_FALSE(validate_json_plain("-2.5", v));
	EXPECT_FALSE(validate_json_plain("-4", v));
	EXPECT_TRUE(validate_json_plain("-5", v));
}

TEST_F(Json, NullAndNumberMultipleOf)
{
	v = parse_schema_bare(
		"{\"type\": [\"null\", \"number\"],"
			"\"multipleOf\": 2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("2", v));
	EXPECT_FALSE(validate_json_plain("3", v));
	EXPECT_TRUE(validate_json_plain("4", v));
}

TEST_F(Json, NullAndIntegerMultipleOf)
{
	v = parse_schema_bare(
		"{\"type\": [\"null\", \"integer\"],"
			"\"multipleOf\": 2.5"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("5", v));
	EXPECT_FALSE(validate_json_plain("2.5", v));
	EXPECT_TRUE(validate_json_plain("10", v));
}

TEST_F(Json, String)
{
	v = parse_schema_bare("{\"type\": \"string\" }");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("\"hello\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, StringMaxLength)
{
	v = parse_schema_bare(
		"{\"type\": \"string\","
			"\"maxLength\": 5"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("\"hello\"", v));
	EXPECT_FALSE(validate_json_plain("\"hello!\"", v));
}

TEST_F(Json, StringMinLength)
{
	v = parse_schema_bare(
		"{\"type\": \"string\","
			"\"minLength\": 6"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("\"hello\"", v));
	EXPECT_TRUE(validate_json_plain("\"hello!\"", v));
	EXPECT_TRUE(validate_json_plain("\"hello!!!\"", v));
}

TEST_F(Json, Array)
{
	v = parse_schema_bare("{\"type\": \"array\" }");
	ASSERT_TRUE(v != NULL);
	ValidationError error;
	EXPECT_FALSE(validate_json("null", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_ARRAY, error.error);
	EXPECT_FALSE(validate_json_plain("false", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("\"hello\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\": null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null, 1, false, \"hello\", {}, {\"a\":null}, [], [null]]", v));

	// some invalid layouts checks
	EXPECT_FALSE(validate_json_plain("]", v));
	EXPECT_FALSE(validate_json_plain("][", v));
}

TEST_F(Json, ArrayGenericSchema)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"items\": { \"type\": \"string\" }"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
	EXPECT_FALSE(validate_json_plain("[1]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\"]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", \"b\", \"c\"]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\", \"b\", null]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\", false, \"c\"]", v));
}

TEST_F(Json, ArraySpecificSchemas)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"items\": [{\"type\": \"number\"}, {}, {\"type\": \"string\"}]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
	EXPECT_TRUE(validate_json_plain("[1]", v));
	EXPECT_TRUE(validate_json_plain("[1, null]", v));
	EXPECT_TRUE(validate_json_plain("[1, \"b\", \"c\"]", v));
	EXPECT_FALSE(validate_json_plain("[1, \"b\", null]", v));
	EXPECT_TRUE(validate_json_plain("[1, false, \"c\", null]", v));
	EXPECT_TRUE(validate_json_plain("[1, false, \"c\", \"d\"]", v));
}

TEST_F(Json, ArrayAdditionalItemsAllowed)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"items\": [{\"type\": \"string\"}, {}],"
			"\"additionalItems\": true"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[1]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\"]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", null]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", 1, null]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", true, \"b\"]", v));
}

TEST_F(Json, ArrayAdditionalItemsDisallowed)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"items\": [{\"type\": \"string\"}, {}],"
			"\"additionalItems\": false"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[1]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\"]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", null]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\", 1, null]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\", true, \"b\"]", v));
}

TEST_F(Json, ArrayAdditionalItemsSpecific)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"items\": [{}, {}],"
			"\"additionalItems\": {\"type\": \"string\"}"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[1]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", null]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\", 1, null]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", true, \"b\"]", v));
	EXPECT_TRUE(validate_json_plain("[\"a\", true, \"b\", \"c\"]", v));
	EXPECT_FALSE(validate_json_plain("[\"a\", true, \"b\", false]", v));
}

TEST_F(Json, ArrayOnlyEmptyAllowed)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"items\": [],"
			"\"additionalItems\": false"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[1]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, ArrayMinItems)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"minItems\": 3"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[1, null]", v));
	EXPECT_TRUE(validate_json_plain("[1, 2, 3]", v));
}

TEST_F(Json, ArrayMaxItems)
{
	v = parse_schema_bare(
		"{\"type\": \"array\","
			"\"maxItems\": 2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[1, null]", v));
	EXPECT_FALSE(validate_json_plain("[1, 2, 3]", v));
}

TEST_F(Json, Object)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"a\": {\"type\": \"null\"}"
			"}"
		"}");
	ASSERT_TRUE(v != NULL);
	ValidationError error;
	EXPECT_FALSE(validate_json("null", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_OBJECT, error.error);
	EXPECT_TRUE(validate_json_plain("{\"a\": null}", v));
	EXPECT_FALSE(validate_json("{\"a\": \"b\"}", v, NULL, &error));
	EXPECT_EQ(VEC_NOT_NULL, error.error);

	// some invalid layout checks
	EXPECT_FALSE(validate_json_plain("}", v));
	EXPECT_FALSE(validate_json_plain("}{", v));
	EXPECT_FALSE(validate_json_plain("{null}", v));
	EXPECT_FALSE(validate_json_plain("{{}}", v));
	EXPECT_FALSE(validate_json_plain("{[}]", v));
	EXPECT_FALSE(validate_json_plain("{\"s\"}", v));
}

TEST_F(Json, ObjectWithKeyword)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"id\": {\"type\": \"null\"}"
			"}"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": null}", v));
	EXPECT_FALSE(validate_json_plain("{\"id\": \"b\"}", v));
}

TEST_F(Json, ObjectAdditionalPropertiesEmpty)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"id\": {\"type\": \"null\"}"
			"}"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": null}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": true}", v));
	EXPECT_TRUE(validate_json_plain("{\"b\": []}", v));
}

TEST_F(Json, ObjectDisallowedAdditionalProperties)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"id\": {\"type\": \"null\"}"
			"},"
			"\"additionalProperties\": false"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": null}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\": true}", v));
	EXPECT_FALSE(validate_json_plain("{\"b\": 5}", v));
}

TEST_F(Json, ObjectAllowedAdditionalProperties)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"id\": {\"type\": \"null\"}"
			"},"
			"\"additionalProperties\": true"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": null}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": true}", v));
	EXPECT_TRUE(validate_json_plain("{\"b\": []}", v));
}

TEST_F(Json, ObjectAdditionalPropertiesSchema)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"id\": {\"type\": \"null\"}"
			"},"
			"\"additionalProperties\": {\"type\": \"null\"}"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": null}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": null}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\": true}", v));
	EXPECT_FALSE(validate_json_plain("{\"b\": \"hello\"}", v));
}

TEST_F(Json, ObjectRequired)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"properties\": {"
				"\"id\": {\"type\": \"number\"},"
				"\"str\": {\"type\": \"string\"}"
			"},"
			"\"required\": [\"id\"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("{\"id\": null}", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": 1}", v));
	EXPECT_FALSE(validate_json_plain("{\"str\": \"hello\"}", v));
	EXPECT_TRUE(validate_json_plain("{\"id\": 1, \"str\": \"hello\"}", v));
	EXPECT_FALSE(validate_json_plain("{\"id\": true, \"str\": \"hello\"}", v));
}

TEST_F(Json, ObjectMaxProperties)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"maxProperties\": 2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": null}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": null, \"b\": 1}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\": null, \"b\": 1, \"c\": false}", v));
}

TEST_F(Json, ObjectMinProperties)
{
	v = parse_schema_bare(
		"{\"type\": \"object\","
			"\"minProperties\": 2"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("{\"a\": null}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": null, \"b\": 1}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\": null, \"b\": 1, \"c\": false}", v));
}

TEST_F(Json, AllOfSchemaSimple)
{
	v = parse_schema_bare("{\"allOf\": [{}]}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
}

TEST_F(Json, AllOfSchemaMultiple)
{
	v = parse_schema_bare("{\"allOf\": [{},{\"type\": \"null\"}]}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, AllOfSchemaAlwaysFailed)
{
	v = parse_schema_bare(
		"{"
			"\"allOf\": ["
				"{\"type\": \"string\"},"
				"{\"type\": \"null\"}"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, AnyOfSchemaSimple)
{
	v = parse_schema_bare(
		"{"
			"\"anyOf\": ["
				"{\"type\": \"null\"}"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, AnyOfSchemaMultipleWithGeneric)
{
	v = parse_schema_bare(
		"{"
			"\"anyOf\": ["
				"{\"type\": \"null\"},"
				"{}"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
}

TEST_F(Json, AnyOfSchemaMultiple)
{
	v = parse_schema_bare(
		"{"
			"\"anyOf\": ["
				"{\"type\": \"null\"},"
				"{\"type\": [\"null\", \"integer\"] }"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, OneOfSchemaSimple)
{
	v = parse_schema_bare("{\"oneOf\": [{}]}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
}

TEST_F(Json, OneOfSchemaMultipleWithGenegic)
{
	v = parse_schema_bare("{\"oneOf\": [{},{\"type\": \"number\"}]}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("1", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("\"a\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
}

TEST_F(Json, OneOfSchemaMultiple)
{
	v = parse_schema_bare(
		"{"
			"\"oneOf\": ["
				"{\"type\": \"null\"},"
				"{\"type\": [\"null\", \"integer\"] }"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, NotSchemaSimple)
{
	v = parse_schema_bare("{\"not\": [{\"type\":\"null\"}]}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("12", v));
	EXPECT_TRUE(validate_json_plain("\"hello\"", v));
	EXPECT_TRUE(validate_json_plain("{}", v));
	EXPECT_TRUE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
}

TEST_F(Json, NotSchemaMultiple)
{
	v = parse_schema_bare(
		"{"
			"\"not\": ["
				"{\"type\":\"null\"},"
				"{\"type\":\"object\"}"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("true", v));
	EXPECT_TRUE(validate_json_plain("12", v));
	EXPECT_TRUE(validate_json_plain("\"hello\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_TRUE(validate_json_plain("[]", v));
	EXPECT_TRUE(validate_json_plain("[null]", v));
}

TEST_F(Json, NotSchemaAllFailed)
{
	v = parse_schema_bare("{\"not\": [{}]}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("12", v));
	EXPECT_FALSE(validate_json_plain("\"hello\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
	EXPECT_FALSE(validate_json_plain("[null]", v));
}

TEST_F(Json, TypeAndAllOfSchema)
{
	v = parse_schema_bare(
		"{"
			"\"type\": \"number\","
			"\"allOf\": ["
				"{},"
				"{\"type\": [\"null\", \"integer\"] }"
			"]"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_FALSE(validate_json_plain("null", v));
	EXPECT_TRUE(validate_json_plain("1", v));
	EXPECT_FALSE(validate_json_plain("1.2", v));
	EXPECT_FALSE(validate_json_plain("true", v));
	EXPECT_FALSE(validate_json_plain("\"a\"", v));
	EXPECT_FALSE(validate_json_plain("{}", v));
	EXPECT_FALSE(validate_json_plain("[]", v));
}

TEST_F(Json, TwoLevelObject)
{
	v = parse_schema_bare(
		"{"
			"\"type\": \"object\","
			"\"properties\": {"
				"\"a\": {"
					"\"type\": \"object\","
					"\"properties\": {"
						"\"b\": {\"type\": \"boolean\"}"
					"}"
				"}"
			"}"
		"}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(validate_json_plain("{\"a\":{\"b\": true}}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":{\"b\": 1}}", v));
	EXPECT_FALSE(validate_json_plain("{\"a\":{\"b\": \"asdf\"}}", v));
}
