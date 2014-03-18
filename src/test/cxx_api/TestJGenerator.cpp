// @@@LICENSE
//
//      Copyright 2012-2013 LG Electronics, Inc.
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

#include <pbnjson.hpp>
#include <pbnjson.h>
#include "gtest/gtest.h"
#include <vector>
#include <string>

namespace pj = pbnjson;

TEST(JGenerator, serialize_with_schema_validation)
{
	{
		pj::JValue json = pj::Object() << pj::JValue(int32_t(1));
		EXPECT_EQ("", pj::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}")));
	}

	{
		pj::JValue json = pj::JValue(int32_t(1));
		EXPECT_EQ("", pj::JGenerator::serialize(json, pbnjson::JSchema::AllSchema()));
	}

	{
		pj::JValue json =  pj::JValue("test");
		EXPECT_EQ("", pj::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}")));
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("bool", false);

		auto schema = pj::JSchemaFragment{
			"{"
				"\"type\":\"object\","
				"\"$schema\": \"http://json-schema.org/draft-03/schema\","
				"\"id\": \"http://jsonschema.net\","
				"\"properties\": {"
					"\"bool\": {"
						"\"type\":\"boolean\","
						"\"id\": \"http://jsonschema.net/int32_t\""
					"}"
				"}"
			"}"
			};

		std::string serialized = pj::JGenerator::serialize(json, schema);
		EXPECT_EQ("{\"bool\":false}", serialized);

		serialized = pj::JGenerator::serialize((*json.begin()).second, pbnjson::JSchemaFragment("{}"));
		EXPECT_EQ("", serialized);

		serialized = pj::JGenerator::serialize(pj::Object() << (*json.begin()), pbnjson::JSchemaFragment("{}"));
		EXPECT_EQ("{\"bool\":false}", serialized);
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("string", "test");
		EXPECT_EQ("{\"string\":\"test\"}", pj::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}")));
	}

	{
		pj::JValue json = pj::Array();
		EXPECT_EQ("[]", pj::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}")));
	}

	{
		pj::JValue json = pj::Array() << pj::JValue(int32_t(1)) << pj::JValue(int32_t(2));
		EXPECT_EQ("[1,2]", pj::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}")));
	}
}

TEST(JGenerator, serialize_without_schema_validation)
{
	{
		pj::JValue json = pj::Array();
		EXPECT_EQ("[]", pj::JGenerator::serialize(json, true));
		json << pj::JValue() << pj::JValue(1) << pj::JValue("test");
		EXPECT_EQ("[null,1,\"test\"]", pj::JGenerator::serialize(json, true));
	}
	{
		pj::JValue json = pj::JValue(int32_t(1));
		EXPECT_EQ("1", pj::JGenerator::serialize(json, true));
	}
	{
		pj::JValue json = pj::JValue(true);
		EXPECT_EQ("true", pj::JGenerator::serialize(json, true));
	}
	{
		pj::JValue json = pj::JValue();
		EXPECT_EQ("null", pj::JGenerator::serialize(json, true));
	}
	{
		pj::JValue json = pj::JValue("");
		EXPECT_EQ("\"\"", pj::JGenerator::serialize(json, true));
		EXPECT_EQ("", pj::JGenerator::serialize(json, false));
	}
	{
		pj::JValue json = pj::JValue("test");
		EXPECT_EQ("\"test\"", pj::JGenerator::serialize(json, true));
		EXPECT_EQ("test", pj::JGenerator::serialize(json, false));
	}
	{
		pj::JValue json = pj::JValue(42323.0234234);
		EXPECT_EQ("42323.0234234", pj::JGenerator::serialize(json, true));
	}
	{
		pj::JValue json = pj::JValue(int64_t(4292496729600));
		EXPECT_EQ("4292496729600", pj::JGenerator::serialize(json, true));
	}
	{
		pj::JValue json = pj::Object();
		EXPECT_EQ("{}", pj::JGenerator::serialize(json, true));
		//Error situation. It is not legal to put JValue, that is not Object or Array, to the Object.
		EXPECT_EQ("null", pj::JGenerator::serialize(json << pj::JValue(int32_t(1)), true));
	}
}
