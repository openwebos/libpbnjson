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

using namespace std;
using namespace pbnjson;

class NOV99444
	: public testing::Test
{
protected:
	string _faulty_schema;
	string _valid_schema;
	string _input;

	virtual void SetUp()
	{
		_faulty_schema =
			"{"
			    "\"type\" : \"object\","
			    "\"properties\" : {"
			        "\"errorCode\" : {"
			            "\"type\" : \"integer\";"
			        "}"
			    "}"
			"}";

		_valid_schema =
			"{"
			    "\"type\" : \"object\","
			    "\"properties\" : {"
			        "\"errorCode\" : {"
			                "\"type\" : \"integer\""
			        "}"
			    "}"
			"}";

		JValue obj = Object()
		           << JValue::KeyValue("returnValue", false)
		           << JValue::KeyValue("errorCode", -1)
		           << JValue::KeyValue("errorText", "Launch helper exited with unknown return code 0");
		ASSERT_TRUE(JGenerator{}.toString(obj, JSchemaFragment("{}"), _input));
	}
};

TEST_F(NOV99444, Success)
{
	JSchema schema = JSchemaFragment(_valid_schema);
	JDomParser parser;

	for (int i = 0; i < 10; i++)
		parser.parse(_input, schema);
	ASSERT_TRUE(parser.parse(_input, schema));

	JValue json = parser.getDom();

	int error_code;
	EXPECT_EQ(CONV_OK, json["errorCode"].asNumber<int>(error_code));
	EXPECT_TRUE(json["errorText"].isString());
}

TEST_F(NOV99444, Failure)
{
	JSchema schema{JSchemaFragment(_faulty_schema)};
	JDomParser parser;

	for (int i = 0; i < 10; i++)
		parser.parse(_input, schema);
	ASSERT_FALSE(parser.parse(_input, schema));
}

TEST(SysmgrFailure, Parse)
{
	// Schema from sysmgr crash
	string valid_schema = "{\"type\":\"object\",\"properties\":{\"quicklaunch\":{\"type\":\"boolean\",\"optional\":true},\"launcher\":{\"type\":\"boolean\",\"optional\":true},\"universal search\":{\"type\":\"boolean\",\"optional\":true}},\"additionalProperties\":false}";
	JValue obj = Object()
	             << JValue::KeyValue("universal search", false)
	             << JValue::KeyValue("launcher", false);
	string input;
	ASSERT_TRUE(JGenerator{}.toString(obj, JSchemaFragment("{}"), input));
	EXPECT_TRUE(JDomParser{}.parse(input, JSchemaFragment{valid_schema}));
}

TEST(GF_7251, MemoryLeak)
{
	const pbnjson::JSchemaFragment schema("{}");

	string serialized;
	ASSERT_TRUE(JGenerator{}.toString(pbnjson::Object(), schema, serialized));
	EXPECT_STREQ("{}", serialized.c_str());
}

TEST(GF_7251, MemoryLeak2)
{
	char const *schema = "{\"type\": \"object\","
	                       "\"properties\": {"
	                         "\"key\": {"
	                           "\"type\" : \"boolean\""
	                         "}"
	                       "}"
	                     "}";
	JValue obj = Object() << JValue::KeyValue("key", "true");
	string serialized;
	ASSERT_FALSE(JGenerator{}.toString(obj, JSchemaFragment{schema}, serialized));
	EXPECT_STREQ("", serialized.c_str());
}

TEST(GF_7251, MemoryLeak3)
{
	char const *schema =
		"{ \"type\": \"object\","
		  "\"properties\": {"
		    "\"hasVideo\":{"
		      "\"type\": \"object\","
		      "\"properties\": {"
		        "\"state\": {\"type\":\"boolean\"},"
		        "\"mediaId\": {\"type\": \"string\"}"
		      "}"
		    "}"
		  "}"
		"}";

	JDomParser parser;
	ASSERT_TRUE(parser.parse("{\"hasVideo\":{\"state\":1,\"mediaId\":\"jtNe8Pn3csslQ6R\"}}", JSchemaFragment("{}")));

	JValue object = parser.getDom();
	string serialized;

	EXPECT_FALSE(JGenerator{}.toString(object, JSchemaFragment{schema}, serialized));
}

// vim: set noet ts=4 sw=4:
