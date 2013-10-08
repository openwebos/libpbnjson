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
#include <pbnjson.hpp>
#include <string>

using namespace std;
using namespace pbnjson;

namespace {


class TestNumberSanity : public ::testing::Test
{
protected:
	static unique_ptr<JSchema> schema;

	static void SetUpTestCase()
	{
		schema.reset(new JSchemaFragment(
			"{"
				"\"type\": \"array\","
				"\"items\": { \"type\": \"number\" },"
				"\"minItems\": 1,"
				"\"maxItems\": 1"
			"}"
			));

		ASSERT_TRUE(schema->isInitialized());
	}
};

unique_ptr<JSchema> TestNumberSanity::schema;

} // namespace

TEST_F(TestNumberSanity, Invalid1)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("[\"abc\"]", *schema.get()));
}

TEST_F(TestNumberSanity, Invalid2)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("[{}]", *schema.get()));
}

TEST_F(TestNumberSanity, Invalid3)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("[]", *schema.get()));
}

TEST_F(TestNumberSanity, Invalid4)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("[true]", *schema.get()));
}

TEST_F(TestNumberSanity, Invalid5)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("[null]", *schema.get()));
}

TEST_F(TestNumberSanity, Invalid6)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", *schema.get()));
}

TEST_F(TestNumberSanity, Valid1)
{
	JDomParser parser;
	EXPECT_TRUE(parser.parse("[1]", *schema.get()));
	JValue parsed = parser.getDom();
	EXPECT_TRUE(parsed.isArray());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}

TEST_F(TestNumberSanity, Valid2)
{
	JDomParser parser;
	EXPECT_TRUE(parser.parse("[1.0]", *schema.get()));
	JValue parsed = parser.getDom();
	EXPECT_TRUE(parsed.isArray());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}

TEST_F(TestNumberSanity, Valid3)
{
	char const *const INPUT =
		"[2394309382309842309234825.62345235323253253220398443213241234"
		"123431413e90234098320982340924382340982349023423498234908234]";

	JDomParser parser;
	EXPECT_TRUE(parser.parse(INPUT, *schema.get()));
	JValue parsed = parser.getDom();
	EXPECT_TRUE(parsed.isArray());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}

TEST_F(TestNumberSanity, Valid4)
{
	JDomParser parser;
	EXPECT_TRUE(parser.parse("[-50]", *schema.get()));
	JValue parsed = parser.getDom();
	EXPECT_TRUE(parsed.isArray());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}

TEST(GF40819, Test)
{
	 pbnjson::JSchemaFile schema("this_file_should_never_exist");
	 EXPECT_FALSE(schema.isInitialized());
}
// vim: set noet ts=4 sw=4 tw=80:
