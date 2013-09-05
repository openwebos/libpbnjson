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

static const string resolution_dir = string{SCHEMA_DIR} + "contact/";

class TestSchemaContact
	: public ::testing::Test
	, public JResolver
{
protected:
	static unique_ptr<JSchema> schema;
	static JSchemaInfo schema_info;
	jvalue_ref parsed;

	static void SetUpTestCase()
	{
		schema.reset(new JSchemaFile(resolution_dir + "Contact.schema"));
		ASSERT_TRUE(schema->isInitialized());
	}

	virtual JSchema resolve(const ResolutionRequest &request,
	                        JSchemaResolutionResult &result)
	{
		string lookup_path = resolution_dir + "/" + request.resource() + ".schema";
		if (-1 == ::access(lookup_path.c_str(), F_OK))
		{
			result = SCHEMA_NOT_FOUND;
			return JSchema::NullSchema();
		}

		JSchemaFile schema{lookup_path.c_str()};
		if (!schema.isInitialized())
		{
			result = SCHEMA_INVALID;
			return JSchema::NullSchema();
		}

		result = SCHEMA_RESOLVED;
		return schema;
	}
};

unique_ptr<JSchema> TestSchemaContact::schema;

} // namespace

TEST_F(TestSchemaContact, Invalid1)
{
	JDomParser parser(this);
	EXPECT_FALSE(parser.parse("", *schema.get()));
}

TEST_F(TestSchemaContact, Valid1)
{
	JDomParser parser(this);
	ASSERT_TRUE(parser.parse(
		"{"
			"\"contactIds\": [ \"1\" ],"
			"\"displayIndex\": \"first name\""
		"}",
		*schema.get()));
	auto parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get(), *this));
}

TEST_F(TestSchemaContact, Valid2)
{
	JDomParser parser(this);
	ASSERT_TRUE(parser.parse("{}", *schema.get()));
	auto parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get(), *this));
}

TEST_F(TestSchemaContact, Valid3)
{
	JDomParser parser(this);
	ASSERT_TRUE(parser.parse(
		"{"
			"\"displayName\": \"\","
			"\"name\": {},"
			"\"birthday\": \"\","
			"\"anniversary\": \"\","
			"\"gender\": \"undisclosed\""
		"}",
		*schema.get())
		);
	auto parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get(), *this));
}

// vim: set noet ts=4 sw=4 tw=80:
