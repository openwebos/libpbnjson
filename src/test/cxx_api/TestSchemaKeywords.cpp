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

TEST(Schemakeywords, DISABLED_extends) {

	pj::JSchemaFile schema("./data/schemas/TestSchemaKeywords/extends/extended.json");
	ASSERT_TRUE(schema.isInitialized());

	myJResolver resolver;

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", 1);
		EXPECT_NE("", pj::JGenerator::serialize(json, schema, &resolver));
	}

	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("iField", "text");
		EXPECT_EQ("", pj::JGenerator::serialize(json, schema, &resolver));
	}
}

} // namespace testcxx
} // namespace pjson
