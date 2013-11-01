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
#include <pbnjson.h>
#include <string>
#include <memory>

using namespace std;

unique_ptr<jvalue, function<void(jvalue_ref &)>> mk_ptr(jvalue_ref v)
{
	unique_ptr<jvalue, function<void(jvalue_ref &)>>
		jv { v, [](jvalue_ref &v) { j_release(&v); } };
	return jv;
}

class TestUniqueItems : public ::testing::Test
{
protected:
	jschema_ref schema;
	JSchemaInfo schema_info;
	JSchemaInfo schema_info_all;

	virtual void SetUp()
	{
		schema = jschema_parse(j_cstr_to_buffer(
			"{"
				"\"type\": \"array\","
				"\"uniqueItems\": true"
			"}"
			), 0, NULL);

		ASSERT_TRUE(schema != NULL);
		jschema_info_init(&schema_info, schema, NULL, NULL);
		jschema_info_init(&schema_info_all, jschema_all(), NULL, NULL);
	}

	virtual void TearDown()
	{
		jschema_release(&schema);
	}
};

TEST_F(TestUniqueItems, Valid0)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_FALSE(jis_null(res.get()));
	EXPECT_TRUE(jvalue_check_schema(res.get(), &schema_info));
}

TEST_F(TestUniqueItems, Valid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_FALSE(jis_null(res.get()));
	EXPECT_TRUE(jvalue_check_schema(res.get(), &schema_info));
}

TEST_F(TestUniqueItems, Valid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_FALSE(jis_null(res.get()));
	EXPECT_TRUE(jvalue_check_schema(res.get(), &schema_info));
}

TEST_F(TestUniqueItems, Valid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true, false]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_FALSE(jis_null(res.get()));
	EXPECT_TRUE(jvalue_check_schema(res.get(), &schema_info));
}

TEST_F(TestUniqueItems, Invalid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true, 1, false, 1]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_TRUE(jis_null(res.get()));
	res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info_all));
	ASSERT_TRUE(jis_array(res.get()));
	EXPECT_FALSE(jvalue_check_schema(res.get(), &schema_info));
}

TEST_F(TestUniqueItems, Invalid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true, 1, true, 0]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_TRUE(jis_null(res.get()));
	res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info_all));
	ASSERT_TRUE(jis_array(res.get()));
	EXPECT_FALSE(jvalue_check_schema(res.get(), &schema_info));
}

TEST_F(TestUniqueItems, Invalid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[{\"a\":1, \"b\":\"hello\"}, true, {\"b\":\"hello\", \"a\":1}, 0]");
	auto res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info));
	EXPECT_TRUE(jis_null(res.get()));
	res = mk_ptr(jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info_all));
	ASSERT_TRUE(jis_array(res.get()));
	EXPECT_FALSE(jvalue_check_schema(res.get(), &schema_info));
}
