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

namespace pjson {
namespace testcxx {
namespace pj = pbnjson;

class myJResolver: public pj::JResolver {
public:
	pj::JSchema resolve(const ResolutionRequest& request, JSchemaResolutionResult& resolutionResult) {

			pj::JSchemaFile mySchema("./data/schemas/TestSchemaKeywords/extends/" + request.resource());

			if (!mySchema.isInitialized()) {
				resolutionResult = SCHEMA_NOT_FOUND;
			}

			resolutionResult = SCHEMA_RESOLVED;
			return mySchema;
	}
};

TEST(Schemakeywords, extends)
{

	pj::JSchemaFile schema("./data/schemas/TestSchemaKeywords/extends/extended.json");
	ASSERT_TRUE(schema.isInitialized());

	myJResolver resolver;

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", "text");
		EXPECT_FALSE(pj::JValidator::isValid(json, schema, resolver));
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", 1);
		EXPECT_TRUE(pj::JValidator::isValid(json, schema, resolver));
	}


}

class ChildResolver : public pbnjson::JResolver
{
public:
	ChildResolver()
	:
	m_schema(pbnjson::JSchemaFile("./data/schemas/TestSchemaKeywords/reuse_schema_in_resolver/child.schema"))
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

	pj::JSchemaFile schema("./data/schemas/TestSchemaKeywords/reuse_schema_in_resolver/parent.schema");
	ASSERT_TRUE(schema.isInitialized());

	ChildResolver resolver;

	std::string data = "{\n";
	data += "\"mychild\" : {\n";
	data += "\"name\" : \"Jone Doe\"\n";
	data += "}\n}";

	pbnjson::JDomParser parser1(&resolver);
	EXPECT_TRUE(parser1.parse(data.c_str(), schema, NULL));
	pbnjson::JDomParser parser2(&resolver);
	EXPECT_TRUE(parser2.parse(data.c_str(), schema, NULL));
}

TEST(JValidator, IsValid_2_params)
{

	pj::JSchemaFile schema("./data/schemas/TestSchemaKeywords/extends/base.json");
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

} // namespace testcxx
} // namespace pjson
