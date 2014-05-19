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

#include <pbnjson.hpp>
#include <pbnjson.h>
#include "gtest/gtest.h"
#include <vector>
#include <stdexcept>
#include <iostream>

using namespace std;

namespace pjson {
namespace testcxx {

namespace pj = pbnjson;

class myJResolver: public pj::JResolver
{
public:
	pj::JSchema resolve(const ResolutionRequest& request, JSchemaResolutionResult& resolutionResult)
	{
		pj::JSchemaFile mySchema(DATA_DIR "TestSchemaKeywords/extends/" + request.resource());

		if (!mySchema.isInitialized())
		{
			resolutionResult = SCHEMA_NOT_FOUND;
		}

		resolutionResult = SCHEMA_RESOLVED;
		return mySchema;
	}
};

TEST(Schemakeywords, extends)
{
	myJResolver resolver;
	pj::JSchemaFile schema(DATA_DIR "TestSchemaKeywords/extends/extended.json", NULL, &resolver);
	ASSERT_TRUE(schema.isInitialized());

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", "text");
		EXPECT_FALSE(pj::JValidator::isValid(json, schema));
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", 1);
		EXPECT_TRUE(pj::JValidator::isValid(json, schema));
	}
}

class ChildResolver : public pbnjson::JResolver
{
public:
	ChildResolver()
		: m_schema(pbnjson::JSchemaFile(DATA_DIR "TestSchemaKeywords/reuse_schema_in_resolver/child.schema"))
	{
	}

	pbnjson::JSchema resolve(const ResolutionRequest &request, JSchemaResolutionResult &result)
	{
		if (request.resource() != "child")
		{
			result = SCHEMA_IO_ERROR;
			return pbnjson::JSchema::NullSchema();
		}

		result = SCHEMA_RESOLVED;
		return m_schema;
	}

private:
	pbnjson::JSchema m_schema;
};


TEST(Schemakeywords, reuse_schema_in_resolver)
{
	ChildResolver resolver;
	pj::JSchemaFile schema(DATA_DIR "TestSchemaKeywords/reuse_schema_in_resolver/parent.schema", NULL, &resolver);
	ASSERT_TRUE(schema.isInitialized());


	std::string data = "{\n";
	data += "\"mychild\" : {\n";
	data += "\"name\" : \"Jone Doe\"\n";
	data += "}\n}";

	pbnjson::JDomParser parser1;
	EXPECT_TRUE(parser1.parse(data.c_str(), schema, NULL));
	pbnjson::JDomParser parser2;
	EXPECT_TRUE(parser2.parse(data.c_str(), schema, NULL));
}

TEST(JValidator, IsValid_2_params)
{
	pj::JSchemaFile schema(DATA_DIR "TestSchemaKeywords/extends/base.json");
	ASSERT_TRUE(schema.isInitialized());

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", 1);
		EXPECT_TRUE(pj::JValidator::isValid(json, schema));
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("sField", "test");
		EXPECT_TRUE(pj::JValidator::isValid(json, schema));
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", true);
		EXPECT_FALSE(pj::JValidator::isValid(json, schema));
	}

}

TEST(JValidator, MinMaxLength)
{
	using namespace pbnjson;

	JSchema schema = JSchemaFragment(
		"{"
			"\"type\" : \"object\","
			"\"properties\" : {"
				"\"a\" : {"
					"\"type\" : \"string\","
					"\"maxLength\" : 5"
				"},"
				"\"b\" : {"
					"\"type\" : \"string\","
					"\"minLength\" : 5"
				"}"
			"}"
		"}"
		);

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("a", "Hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("a", "Hello, world!"), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("b", "Hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("b", "Hell"), schema));
}

TEST(JValidator, AllowedTypes)
{
	using namespace pbnjson;

	JSchema schema = JSchemaFragment(
		"{"
			"\"type\" : \"object\","
			"\"properties\" : {"
				"\"str\" : {"
					"\"type\" : \"string\""
				"},"
				"\"bool\" : {"
					"\"type\" : \"boolean\""
				"},"
				"\"num\" : {"
					"\"type\" : \"number\""
				"},"
				"\"int\" : {"
					"\"type\" : \"integer\""
				"},"
				"\"null\" : {"
					"\"type\" : \"null\""
				"},"
				"\"arr\" : {"
					"\"type\" : \"array\""
				"},"
				"\"obj\" : {"
					"\"type\" : \"object\""
				"},"
				"\"strOrNull\" : {"
					"\"type\" : [\"string\", \"null\"]"
				"},"
				"\"any\" : {"
					"\"type\" : \"any\""
				"}"
			"}"
		"}"
		);

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("str", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str", true), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str", 2.3), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str", Object()), schema));

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("bool", "hello"), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("bool", true), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("bool", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("bool", 2.3), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("bool", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("bool", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("bool", Object()), schema));

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("num", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("num", true), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("num", 1), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("num", 2.3), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("num", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("num", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("num", Object()), schema));

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("int", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("int", true), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("int", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("int", 2.3), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("int", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("int", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("int", Object()), schema));

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("null", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("null", true), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("null", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("null", 2.3), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("null", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("null", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("null", Object()), schema));

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("arr", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("arr", true), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("arr", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("arr", 2.3), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("arr", JValue()), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("arr", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("arr", Object()), schema));

	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("obj", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("obj", true), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("obj", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("obj", 2.3), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("obj", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("obj", Array()), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("obj", Object()), schema));

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", true), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", 1), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", 2.3), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", JValue()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", Array()), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("strOrNull", Object()), schema));

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", "hello"), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", true), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", 1), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", 2.3), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", JValue()), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", Array()), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("any", Object()), schema));
}

TEST(JValidator, AdditionalPropertiesDisallowed)
{
	using namespace pbnjson;

	JSchema schema = JSchemaFragment(
		"{"
			"\"type\" : \"object\","
			"\"properties\" : {"
				"\"str\" : {}"
			"},"
			"\"additionalProperties\" : false"
		"}"
		);

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("str", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("str2", "hello"), schema));
}

TEST(JValidator, AdditionalPropertiesAllowed)
{
	using namespace pbnjson;

	JSchema schema = JSchemaFragment(
		"{"
			"\"type\" : \"object\","
			"\"properties\" : {"
				"\"str\" : {}"
			"},"
			"\"additionalProperties\" : true"
		"}"
		);

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("str", "hello"), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("str2", "hello"), schema));
}

TEST(JValidator, AdditionalPropertiesSchema)
{
	using namespace pbnjson;

	JSchema schema = JSchemaFragment(
		"{"
			"\"type\" : \"object\","
			"\"properties\" : {"
				"\"str\" : {}"
			"},"
			"\"additionalProperties\" : {"
				"\"type\" : \"object\","
				"\"properties\" : {"
					"\"num\" : {"
						"\"type\" : \"number\""
					"}"
				"}"
			"}"
		"}"
		);

	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("str", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("add", "hello"), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("add", 1), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("add", Object()), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("add", Object() << JValue::KeyValue("num", 1)), schema));
	EXPECT_FALSE(JValidator::isValid(Object() << JValue::KeyValue("add", Object() << JValue::KeyValue("num", "hello")), schema));
	EXPECT_TRUE(JValidator::isValid(Object() << JValue::KeyValue("add", Object() << JValue::KeyValue("str", "hello")), schema));
}

TEST(JValidator, DefaultProperties)
{
	using namespace pbnjson;

	auto schema = JSchemaFragment{
		"{"
			"\"type\": \"object\","
			"\"properties\": {"
				"\"a\": {\"type\": \"string\", \"default\": \"hello\"},"
				"\"b\": {\"type\": \"number\", \"default\": 3.14},"
				"\"c\": {\"type\": \"boolean\", \"default\": true},"
				"\"d\": {\"default\": null},"
				"\"e\": {"
					"\"type\": \"object\","
					"\"properties\": {"
						"\"e1\": {\"type\": \"string\", \"default\": \"asd\"},"
						"\"e2\": {\"type\": \"number\", \"default\": 2}"
					"}"
				"}"
			"}"
		"}"
		};

	JDomParser parser;
	ASSERT_TRUE(parser.parse("{\"a\": \"qwer\"}", schema, NULL)) << parser.getError();
	auto val = parser.getDom();
	ASSERT_TRUE(val.isObject());
	ASSERT_TRUE(val.hasKey("a"));
	EXPECT_EQ(val["a"], "qwer");
	ASSERT_TRUE(val.hasKey("b"));
	EXPECT_EQ(val["b"], 3.14);
	ASSERT_TRUE(val.hasKey("c"));
	EXPECT_EQ(val["c"], true);
	ASSERT_TRUE(val.hasKey("d"));
	EXPECT_TRUE(val["d"].isNull());

	ASSERT_TRUE(parser.parse("{\"a\": \"qwer\", \"e\":{}}", JSchema::AllSchema(), NULL));
	JValue val_all_schema = parser.getDom();
	ASSERT_TRUE(val_all_schema.isObject());
	ASSERT_TRUE(val_all_schema.hasKey("a"));
	ASSERT_FALSE(val_all_schema.hasKey("b"));
	ASSERT_FALSE(val_all_schema.hasKey("c"));
	ASSERT_FALSE(val_all_schema.hasKey("d"));

	ASSERT_TRUE(JValidator::apply(val_all_schema, schema));
	ASSERT_TRUE(val_all_schema.hasKey("a"));
	EXPECT_EQ(val_all_schema["a"], "qwer");
	ASSERT_TRUE(val_all_schema.hasKey("b"));
	EXPECT_EQ(val_all_schema["b"], 3.14);
	ASSERT_TRUE(val_all_schema.hasKey("c"));
	EXPECT_EQ(val_all_schema["c"], true);
	ASSERT_TRUE(val_all_schema.hasKey("d"));
	EXPECT_TRUE(val_all_schema["d"].isNull());
	ASSERT_TRUE(val_all_schema.hasKey("e"));
	ASSERT_TRUE(val_all_schema["e"].hasKey("e1"));
	ASSERT_TRUE(val_all_schema["e"].hasKey("e2"));
}

} // namespace testcxx
} // namespace pjson
