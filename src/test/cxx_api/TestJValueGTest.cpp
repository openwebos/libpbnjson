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

namespace pjson {
namespace testcxx {
namespace pj = pbnjson;


TEST(JValue, hasKey) {
	{
		pj::JValue json;
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey(""));
	}
	{
		pj::JValue json = pj::Object();
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey(""));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue(int32_t(1)) <<
										  pj::JValue(int64_t(1)) <<
										  pj::JValue(1.0) <<
										  pj::JValue("string") <<
										  pj::JValue(true);
		EXPECT_FALSE(json.hasKey("1"));
		EXPECT_FALSE(json.hasKey("1.0"));
		EXPECT_FALSE(json.hasKey("string"));
		EXPECT_FALSE(json.hasKey("true"));
		EXPECT_FALSE(json.hasKey(""));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("null", pj::JValue());
		EXPECT_TRUE(json.hasKey("null"));
		EXPECT_FALSE(json.hasKey("JNULL"));
		EXPECT_FALSE(json.hasKey(""));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("int32_t", int32_t(1));
		EXPECT_TRUE(json.hasKey("int32_t"));
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey("1"));
		EXPECT_FALSE(json.hasKey(""));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("int64_t", int64_t(1));
		EXPECT_TRUE(json.hasKey("int64_t"));
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey("1"));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("double", 1.0);
		EXPECT_TRUE(json.hasKey("double"));
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey("1.0"));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("string", "test");
		EXPECT_TRUE(json.hasKey("string"));
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey("test"));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("bool", true);
		EXPECT_TRUE(json.hasKey("bool"));
		EXPECT_FALSE(json.hasKey("foo"));
		EXPECT_FALSE(json.hasKey("true"));
	}
	{
		pj::JValue json = pj::Array();
		json.append(int32_t(1));
		json.append("test string");
		EXPECT_FALSE(json.hasKey("1"));
		EXPECT_FALSE(json[0].hasKey("1"));
		EXPECT_FALSE(json[0].hasKey("test string"));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("int32_t", int32_t(1)) <<
										  pj::JValue::KeyValue("int64_t", int64_t(1)) <<
										  pj::JValue::KeyValue("double", 1.0) <<
										  pj::JValue::KeyValue("string", "test") <<
										  pj::JValue::KeyValue("bool", true);
		EXPECT_TRUE(json.hasKey("int32_t"));
		EXPECT_TRUE(json.hasKey("int64_t"));
		EXPECT_TRUE(json.hasKey("double"));
		EXPECT_TRUE(json.hasKey("string"));
		EXPECT_TRUE(json.hasKey("bool"));
		EXPECT_FALSE(json.hasKey("1"));
		EXPECT_FALSE(json.hasKey("1.0"));
		EXPECT_FALSE(json.hasKey("test"));
		EXPECT_FALSE(json.hasKey("true"));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("key with space", "test") <<
										  pj::JValue::KeyValue("space at the end ", "test");
		EXPECT_TRUE(json.hasKey("key with space"));
		EXPECT_FALSE(json.hasKey("key"));
		EXPECT_TRUE(json.hasKey("space at the end "));
		EXPECT_FALSE(json.hasKey("space at the end"));
	}
	{
		pj::JValue json = pj::Object() << pj::JValue::KeyValue("\"", 1) <<
										  pj::JValue::KeyValue("b", 1) <<
										  pj::JValue::KeyValue("f", 1) <<
										  pj::JValue::KeyValue("n", 1) <<
										  pj::JValue::KeyValue("r", 1) <<
										  pj::JValue::KeyValue("t", 1);
		EXPECT_TRUE(json.hasKey("\""));
		EXPECT_TRUE(json.hasKey("b"));
		EXPECT_TRUE(json.hasKey("f"));
		EXPECT_TRUE(json.hasKey("n"));
		EXPECT_TRUE(json.hasKey("r"));
		EXPECT_TRUE(json.hasKey("t"));
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


} // namespace testcxx
} // namespace pjson

