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
#include "../generic_validator.h"
#include "../null_validator.h"
#include <gtest/gtest.h>

using namespace std;

class Parser : public ::testing::Test
{
protected:
	Validator *v;
	// Error info
	size_t offset;
	string message;

	virtual void SetUp()
	{
		v = NULL;
		offset = -1;
		message = "";
	}

	virtual void TearDown()
	{
		validator_unref(v), v = NULL;
	}

	static void OnError(size_t offset, char const *message, void *ctxt)
	{
		Parser *args = reinterpret_cast<Parser *>(ctxt);
		args->offset = offset;
		args->message = message;
	}

};

TEST_F(Parser, Empty)
{
	v = parse_schema_bare("{}");
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(GENERIC_VALIDATOR == v);
}

TEST_F(Parser, Empty2)
{
	char const *const SCHEMA =
		"{"
			"\"title\": \"Hello\","
			"\"$schema\": \"http://openwebosproject.org/schema#\""
		"}";
	v = parse_schema_bare(SCHEMA);
	ASSERT_TRUE(v != NULL);
	EXPECT_EQ(GENERIC_VALIDATOR->vtable, v->vtable);
}

TEST_F(Parser, Null)
{
	char const *const SCHEMA = "{\"type\": \"null\"}";
	v = parse_schema_bare(SCHEMA);
	ASSERT_TRUE(v != NULL);
	EXPECT_TRUE(NULL_VALIDATOR == v);
}

TEST_F(Parser, SyntaxError)
{
	v = parse_schema_no_uri("{\"type\": \"null\",}", OnError, this);
	EXPECT_TRUE(v == NULL);

	EXPECT_EQ(17, offset);
	EXPECT_EQ("parse error: invalid object key (must be a string)\n", message);
}

TEST_F(Parser, TypeError)
{
	v = parse_schema_no_uri("{\"type\": \"asdf\"}", OnError, this);
	EXPECT_TRUE(v == NULL);

	EXPECT_EQ(16, offset);
	EXPECT_EQ("Invalid type", message);
}

TEST_F(Parser, MaxLengthNegative)
{
	v = parse_schema_no_uri("{\"maxLength\": -1}", OnError, this);
	EXPECT_TRUE(v == NULL);

	EXPECT_EQ(17, offset);
	EXPECT_EQ("Invalid maxLength format", message);
}

TEST_F(Parser, MaxLengthFloat)
{
	v = parse_schema_no_uri("{\"maxLength\": 4.2}", OnError, this);
	EXPECT_TRUE(v == NULL);

	EXPECT_EQ(18, offset);
	EXPECT_EQ("Invalid maxLength format", message);
}

TEST_F(Parser, MinLengthNegative)
{
	v = parse_schema_no_uri("{\"minLength\": -1}", OnError, this);
	EXPECT_TRUE(v == NULL);

	EXPECT_EQ(17, offset);
	EXPECT_EQ("Invalid minLength format", message);
}

TEST_F(Parser, MinLengthFloat)
{
	v = parse_schema_no_uri("{\"minLength\": 4.2}", OnError, this);
	EXPECT_TRUE(v == NULL);

	EXPECT_EQ(18, offset);
	EXPECT_EQ("Invalid minLength format", message);
}
