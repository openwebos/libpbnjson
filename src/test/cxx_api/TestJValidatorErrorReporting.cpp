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

using namespace pbnjson;
using namespace std;

class TestJValidatorErrorReporting : public ::testing::Test, public JErrorHandler
{
protected:
	unsigned int errorCounter;
	int errorCode;

	virtual void SetUp()
	{
		errorCounter = 0;
		errorCode = 0;
	}

	virtual void schema(JParser *, SchemaError error, const string &desc)
	{
		++errorCounter;
		errorCode = error;
	};

	virtual void syntax(JParser *, SyntaxError, const string &) {};
	virtual void misc(JParser *, const string &reason) {};
	virtual void parseFailed(JParser *, const string &) {};
};

TEST_F(TestJValidatorErrorReporting, Array)
{
	JSchema schema = JSchemaFragment("{\"type\": \"array\", \"items\": [{\"type\": \"null\"}], \"additionalItems\": false }");

	EXPECT_FALSE(JValidator::isValid(Array() << JValue(1), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Array() << JValue("hello"), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Array() << JValue(false), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Array() << Array(), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Array() << Object(), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Array() << JValue() << JValue(), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_GENERIC, errorCode);
}

TEST_F(TestJValidatorErrorReporting, Object)
{
	JSchema schema = JSchemaFragment(
		"{"
			"\"type\": \"object\", "
			"\"properties\": {\"a\": {}, \"b\": {\"type\": \"array\"} }, "
			"\"required\": [\"a\"], "
			"\"additionalProperties\": false "
		"}"
	);

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", JValue()) << JValue::KeyValue("b", JValue()), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", JValue()) << JValue::KeyValue("b", 1), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", JValue()) << JValue::KeyValue("b", false), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", JValue()) << JValue::KeyValue("b", "hello"), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", JValue()) << JValue::KeyValue("b", Object()), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_UNEXPECTED_TYPE, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("b", Array()), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_MISSING_REQUIRED_KEY, errorCode);

	SetUp();
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", JValue()) << JValue::KeyValue("c", Array()), schema, this));
	EXPECT_EQ(1, errorCounter);
	EXPECT_EQ(JErrorHandler::ERR_SCHEMA_GENERIC, errorCode);
}
