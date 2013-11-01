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

#include "../validation_api.h"
#include "../parser_api.h"
#include "Util.hpp"
#include <gtest/gtest.h>

using namespace std;

TEST(TestEnum, NullTrue)
{
	char const *const SCHEMA = "{ \"enum\": [null, true] }";
	auto v = mk_ptr(parse_schema_bare(SCHEMA), validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_TRUE(validate_json_plain("null", v.get()));
	EXPECT_TRUE(validate_json_plain("true", v.get()));
	EXPECT_FALSE(validate_json_plain("false", v.get()));
	EXPECT_FALSE(validate_json_plain("1", v.get()));
	EXPECT_FALSE(validate_json_plain("\"asdf\"", v.get()));
	EXPECT_FALSE(validate_json_plain("{}", v.get()));
	EXPECT_FALSE(validate_json_plain("[]", v.get()));
}

TEST(TestEnum, Strings)
{
	char const *const SCHEMA = "{ \"enum\": [\"red\", \"green\", \"blue\"] }";
	auto v = mk_ptr(parse_schema_bare(SCHEMA), validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_FALSE(validate_json_plain("null", v.get()));
	EXPECT_FALSE(validate_json_plain("true", v.get()));
	EXPECT_FALSE(validate_json_plain("false", v.get()));
	EXPECT_FALSE(validate_json_plain("1", v.get()));
	EXPECT_FALSE(validate_json_plain("\"asdf\"", v.get()));
	EXPECT_FALSE(validate_json_plain("{}", v.get()));
	EXPECT_FALSE(validate_json_plain("[]", v.get()));

	EXPECT_TRUE(validate_json_plain("\"red\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"green\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"blue\"", v.get()));

	EXPECT_FALSE(validate_json_plain("\"black\"", v.get()));
}

TEST(TestEnum, Numbers)
{
	char const *const SCHEMA = "{ \"enum\": [0, 3.14, -5012349872340987234] }";
	auto v = mk_ptr(parse_schema_bare(SCHEMA), validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_FALSE(validate_json_plain("null", v.get()));
	EXPECT_FALSE(validate_json_plain("true", v.get()));
	EXPECT_FALSE(validate_json_plain("false", v.get()));
	EXPECT_FALSE(validate_json_plain("1", v.get()));
	EXPECT_FALSE(validate_json_plain("\"asdf\"", v.get()));
	EXPECT_FALSE(validate_json_plain("{}", v.get()));
	EXPECT_FALSE(validate_json_plain("[]", v.get()));

	EXPECT_TRUE(validate_json_plain("0", v.get()));
	EXPECT_TRUE(validate_json_plain("3.14", v.get()));
	EXPECT_TRUE(validate_json_plain("-5012349872340987234", v.get()));
}

TEST(TestEnum, Arrays)
{
	char const *const SCHEMA = "{ \"enum\": [[], [3.14, \"abc\"], [null, false]] }";
	auto v = mk_ptr(parse_schema_bare(SCHEMA), validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_FALSE(validate_json_plain("null", v.get()));
	EXPECT_FALSE(validate_json_plain("true", v.get()));
	EXPECT_FALSE(validate_json_plain("false", v.get()));
	EXPECT_FALSE(validate_json_plain("1", v.get()));
	EXPECT_FALSE(validate_json_plain("\"asdf\"", v.get()));
	EXPECT_FALSE(validate_json_plain("{}", v.get()));

	EXPECT_TRUE(validate_json_plain("[]", v.get()));
	EXPECT_TRUE(validate_json_plain("[3.14, \"abc\"]", v.get()));
	EXPECT_FALSE(validate_json_plain("[3.14]", v.get()));
	EXPECT_TRUE(validate_json_plain("[null, false]", v.get()));
	EXPECT_FALSE(validate_json_plain("[null, false, true]", v.get()));
}

TEST(TestEnum, Objects)
{
	char const *const SCHEMA = "{ \"enum\": [{}, {\"a\":0, \"b\":true}, {\"c\":{\"i\":{}}}] }";
	auto v = mk_ptr(parse_schema_bare(SCHEMA), validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_FALSE(validate_json_plain("null", v.get()));
	EXPECT_FALSE(validate_json_plain("true", v.get()));
	EXPECT_FALSE(validate_json_plain("false", v.get()));
	EXPECT_FALSE(validate_json_plain("1", v.get()));
	EXPECT_FALSE(validate_json_plain("\"asdf\"", v.get()));
	EXPECT_FALSE(validate_json_plain("[]", v.get()));

	EXPECT_TRUE(validate_json_plain("{}", v.get()));
	EXPECT_TRUE(validate_json_plain("{\"b\":true, \"a\":0}", v.get()));
	EXPECT_TRUE(validate_json_plain("{\"c\":{\"i\":{}}}", v.get()));
	EXPECT_FALSE(validate_json_plain("{\"a\":0}", v.get()));
	EXPECT_FALSE(validate_json_plain("{\"a\":0, \"b\":true, \"c\": null}", v.get()));
}

TEST(TestEnum, StringWithArrayInEnum)
{
	char const *const SCHEMA =
		"{"
			"\"enum\": ["
				"\"Dummy\","
				"\"Array\","
				"\"ArrayExt\","
				"\"Range\","
				"\"Date\","
				"\"Callback\","
				"\"File\""
				"]"
		"}";
	auto v = mk_ptr(parse_schema_bare(SCHEMA), validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_TRUE(validate_json_plain("\"Array\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"ArrayExt\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"Dummy\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"Range\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"Date\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"Callback\"", v.get()));
	EXPECT_TRUE(validate_json_plain("\"File\"", v.get()));

	EXPECT_FALSE(validate_json_plain("\"array\"", v.get()));
	EXPECT_FALSE(validate_json_plain("\"file\"", v.get()));
}
