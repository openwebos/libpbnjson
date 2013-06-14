// @@@LICENSE
//
//      Copyright (c) 2013 LG Electronics, Inc.
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

TEST(TestJArray, TestEqual)
{
	auto create_jval = []()
	{
		pbnjson::JValue j_root = pbnjson::Object();
		pbnjson::JValue j_child = pbnjson::Array();
		pbnjson::JValue j_child_of_child = pbnjson::Object();

		j_child << j_child_of_child;
		j_root.put("child", j_child);

		return j_root;
	};

	pbnjson::JValue j_obj1 = create_jval();
	pbnjson::JValue j_obj2 = create_jval();

	ASSERT_TRUE(j_obj1 == j_obj2);
}

TEST(TestJArray, TestEqual2)
{
	using namespace pbnjson;

	string input1 =
		"{\"subscribed\":false,\"mime\":\"audio/mpeg\",\"verb\":\"exampleVerb\","
		"\"returnValue\":true,\"resourceHandlers\":{\"alternates\":"
		"[{\"mime\":\"audio/mpeg\",\"extension\":\"mp3\","
		"\"appId\":\"com.yourdomain.helloworld\",\"streamable\":true,\"index\":1,"
		"\"verbs\":{\"exampleVerb\":\"42\"}}]}}";
	string input2 =
		"{\"subscribed\":false,\"mime\":\"audio/mpeg\",\"verb\":\"exampleVerb\","
		"\"returnValue\":true,\"resourceHandlers\":{\"alternates\":"
		"[{\"mime\":\"audio/mpeg\",\"extension\":\"mp3\","
		"\"appId\":\"com.yourdomain.helloworld\",\"streamable\":true,\"index\":1,"
		"\"exampleVerb\":\"42\",\"verbs\":{}}]}}";

	JDomParser parser(NULL);
	ASSERT_TRUE(parser.parse(input1, JSchema::AllSchema(), NULL));
	pbnjson::JValue val1 = parser.getDom();
	ASSERT_TRUE(parser.parse(input2, JSchema::AllSchema(), NULL));
	pbnjson::JValue val2 = parser.getDom();

	EXPECT_FALSE(val1 == val2);
}
