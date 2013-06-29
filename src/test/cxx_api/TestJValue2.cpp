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

using namespace pbnjson;
using namespace std;

TEST(TestJValue, CopyConstructor)
{
	JValue v1(Object());
	v1.put("key1", "val1");
	JValue v2(v1);
	v2.put("key2", "val2");

	EXPECT_TRUE(v1 == v2);
}

TEST(TestJValue, Duplicate)
{
	JValue v1(Object());
	v1.put("key1", "val1");
	JValue v2(v1.duplicate());
	v2.put("key2", "val2");

	EXPECT_TRUE(v1 != v2);
	EXPECT_TRUE(v1.hasKey("key1"));
	EXPECT_FALSE(v1.hasKey("key2"));
	EXPECT_TRUE(v2.hasKey("key1"));
	EXPECT_TRUE(v2.hasKey("key2"));
}

TEST(TestJValue, IteratorAdvance)
{
	JValue obj = Object();
	EXPECT_TRUE(obj.objectSize() == 0);
	for (int i = 0; i != 10; ++i)
	{
		char key = 'a' + i;
		obj.put(string(&key, &key + 1), JValue(i));
	}
	ASSERT_EQ(obj.objectSize(), 10);

	JValue::ObjectIterator it1 = obj.begin();
	for (int i = 1; i != 5; ++i)
	{
		JValue::KeyValue kv1 = *++it1;
		JValue::KeyValue kv2 = *(obj.begin() + i);
		EXPECT_TRUE(kv1 == kv2);
	}
}
