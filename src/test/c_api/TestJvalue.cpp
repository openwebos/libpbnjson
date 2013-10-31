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
#include <pbnjson.h>
#include <memory>

using namespace std;

unique_ptr<jvalue, function<void(jvalue_ref &)>> mk_ptr(jvalue_ref v)
{
	unique_ptr<jvalue, function<void(jvalue_ref &)>>
		jv { v, [](jvalue_ref &v) { j_release(&v); } };
	return jv;
}

class JvalueEqual
	: public testing::Test
{
protected:
	virtual void SetUp()
	{
		null = jnull();
		boolean = jboolean_create(false);
		str = jstring_create("hello");
		num = jnumber_create(J_CSTR_TO_BUF("0"));
		arr = jarray_create(NULL);
		obj = jobject_create();
	}

	virtual void TearDown()
	{
		j_release(&boolean);
		j_release(&str);
		j_release(&num);
		j_release(&arr);
		j_release(&obj);
	}

	jvalue_ref null;
	jvalue_ref boolean;
	jvalue_ref str;
	jvalue_ref num;
	jvalue_ref arr;
	jvalue_ref obj;
};

TEST_F(JvalueEqual, Null)
{
	jvalue_ref null2 = jnull();

	ASSERT_TRUE(jvalue_equal(null, null));
	ASSERT_TRUE(jvalue_equal(null, null2));
	ASSERT_FALSE(jvalue_equal(null, boolean));
	ASSERT_FALSE(jvalue_equal(null, str));
	ASSERT_FALSE(jvalue_equal(null, num));
	ASSERT_FALSE(jvalue_equal(null, arr));
	ASSERT_FALSE(jvalue_equal(null, obj));
}

TEST_F(JvalueEqual, Boolean)
{
	auto boolean2 = mk_ptr(jboolean_create(false));
	auto boolean3 = mk_ptr(jboolean_create(true));

	ASSERT_TRUE(jvalue_equal(boolean, boolean));
	ASSERT_TRUE(jvalue_equal(boolean, boolean2.get()));
	ASSERT_FALSE(jvalue_equal(boolean, boolean3.get()));
	ASSERT_FALSE(jvalue_equal(boolean, null));
	ASSERT_FALSE(jvalue_equal(boolean, str));
	ASSERT_FALSE(jvalue_equal(boolean, num));
	ASSERT_FALSE(jvalue_equal(boolean, arr));
	ASSERT_FALSE(jvalue_equal(boolean, obj));
}

TEST_F(JvalueEqual, String)
{
	auto str2 = mk_ptr(jstring_create("hello"));
	auto str3 = mk_ptr(jstring_create("world"));

	ASSERT_TRUE(jvalue_equal(str, str));
	ASSERT_TRUE(jvalue_equal(str, str2.get()));
	ASSERT_FALSE(jvalue_equal(str, str3.get()));
	ASSERT_FALSE(jvalue_equal(str, null));
	ASSERT_FALSE(jvalue_equal(str, boolean));
	ASSERT_FALSE(jvalue_equal(str, num));
	ASSERT_FALSE(jvalue_equal(str, arr));
	ASSERT_FALSE(jvalue_equal(str, obj));
}

TEST_F(JvalueEqual, Number)
{
	auto num2 = mk_ptr(jnumber_create(J_CSTR_TO_BUF("0")));
	auto num3 = mk_ptr(jnumber_create(J_CSTR_TO_BUF("0.1")));
	auto num4 = mk_ptr(jnumber_create_i32(0));
	auto num5 = mk_ptr(jnumber_create_i32(1));

	ASSERT_TRUE(jvalue_equal(num, num));
	ASSERT_TRUE(jvalue_equal(num, num2.get()));
	ASSERT_FALSE(jvalue_equal(num, num3.get()));
	ASSERT_TRUE(jvalue_equal(num, num4.get()));
	ASSERT_FALSE(jvalue_equal(num, num5.get()));
	ASSERT_FALSE(jvalue_equal(num, null));
	ASSERT_FALSE(jvalue_equal(num, boolean));
	ASSERT_FALSE(jvalue_equal(num, str));
	ASSERT_FALSE(jvalue_equal(num, arr));
	ASSERT_FALSE(jvalue_equal(num, obj));
}

TEST_F(JvalueEqual, Array)
{
	jarray_append(arr, jnull());
	auto arr2 = mk_ptr(jarray_create(NULL));
	auto arr3 = mk_ptr(jarray_create(NULL));
	jarray_append(arr3.get(), jnull());
	auto arr4 = mk_ptr(jarray_create(NULL));
	jarray_append(arr4.get(), jarray_create(NULL));
	auto arr5 = mk_ptr(jarray_create(NULL));
	jarray_append(arr5.get(), jnull());
	jarray_append(arr5.get(), jstring_create("hello"));

	ASSERT_TRUE(jvalue_equal(arr, arr));
	ASSERT_FALSE(jvalue_equal(arr, arr2.get()));
	ASSERT_TRUE(jvalue_equal(arr, arr3.get()));
	ASSERT_FALSE(jvalue_equal(arr, arr4.get()));
	ASSERT_FALSE(jvalue_equal(arr, arr5.get()));
	ASSERT_FALSE(jvalue_equal(arr, null));
	ASSERT_FALSE(jvalue_equal(arr, boolean));
	ASSERT_FALSE(jvalue_equal(arr, str));
	ASSERT_FALSE(jvalue_equal(arr, num));
	ASSERT_FALSE(jvalue_equal(arr, obj));
}

TEST_F(JvalueEqual, Object)
{
	jobject_put(obj, J_CSTR_TO_JVAL("a"), jnumber_create_i32(0));
	jobject_put(obj, J_CSTR_TO_JVAL("b"), jstring_create("hello"));
	auto obj2 = mk_ptr(jobject_create());
	jobject_put(obj2.get(), J_CSTR_TO_JVAL("a"), jnumber_create_i32(0));
	auto obj3 = mk_ptr(jobject_create());
	jobject_put(obj3.get(), J_CSTR_TO_JVAL("b"), jstring_create("hello"));
	jobject_put(obj3.get(), J_CSTR_TO_JVAL("a"), jnumber_create_i32(0));
	auto obj4 = mk_ptr(jobject_create());
	jobject_put(obj4.get(), J_CSTR_TO_JVAL("a"), jnumber_create_i32(0));
	jobject_put(obj4.get(), J_CSTR_TO_JVAL("b"), jstring_create("world"));
	auto obj5 = mk_ptr(jobject_create());
	jobject_put(obj5.get(), J_CSTR_TO_JVAL("a"), jnumber_create_i32(1));
	jobject_put(obj5.get(), J_CSTR_TO_JVAL("b"), jstring_create("hello"));
	auto obj6 = mk_ptr(jobject_create());
	jobject_put(obj6.get(), J_CSTR_TO_JVAL("a"), jnumber_create_i32(0));
	jobject_put(obj6.get(), J_CSTR_TO_JVAL("b"), jstring_create("hello"));
	jobject_put(obj6.get(), J_CSTR_TO_JVAL("c"), jnull());

	ASSERT_TRUE(jvalue_equal(obj, obj));
	ASSERT_FALSE(jvalue_equal(obj, obj2.get()));
	ASSERT_TRUE(jvalue_equal(obj, obj3.get()));
	ASSERT_FALSE(jvalue_equal(obj, obj4.get()));
	ASSERT_FALSE(jvalue_equal(obj, obj5.get()));
	ASSERT_FALSE(jvalue_equal(obj, obj6.get()));
	ASSERT_FALSE(jvalue_equal(obj, null));
	ASSERT_FALSE(jvalue_equal(obj, boolean));
	ASSERT_FALSE(jvalue_equal(obj, str));
	ASSERT_FALSE(jvalue_equal(obj, num));
	ASSERT_FALSE(jvalue_equal(obj, arr));
}
