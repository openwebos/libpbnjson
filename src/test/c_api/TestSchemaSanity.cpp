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

using namespace std;

namespace {


class TestNumberSanity : public ::testing::Test
{
protected:
	static jschema_ref schema;
	static JSchemaInfo schema_info;
	jvalue_ref parsed;

	static void SetUpTestCase()
	{
		schema = jschema_parse(j_cstr_to_buffer(
			"{"
				"\"type\": \"array\","
				"\"items\": { \"type\": \"number\" },"
				"\"minItems\": 1,"
				"\"maxItems\": 1"
			"}"
			), 0, NULL);

		ASSERT_TRUE(schema != NULL);
		jschema_info_init(&schema_info, schema, NULL, NULL);
	}

	static void TearDownTestCase()
	{
		jschema_release(&schema);
	}

	virtual void SetUp()
	{
		parsed = NULL;
	}

	virtual void TearDown()
	{
		j_release(&parsed);
	}
};

jschema_ref TestNumberSanity::schema = NULL;
JSchemaInfo TestNumberSanity::schema_info;

} // namespace

TEST_F(TestNumberSanity, Invalid0)
{
	const raw_buffer INPUT = j_cstr_to_buffer(" ");

	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
}

TEST_F(TestNumberSanity, Invalid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[\"abc\"]");

	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
}

TEST_F(TestNumberSanity, Invalid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[{}]");

	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
}

TEST_F(TestNumberSanity, Invalid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[]");
	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
}

TEST_F(TestNumberSanity, Invalid4)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[true]");
	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
}

TEST_F(TestNumberSanity, Invalid5)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null]");
	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
}

TEST_F(TestNumberSanity, Invalid6)
{
	const raw_buffer INPUT = j_cstr_to_buffer("{}");
	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
}

TEST_F(TestNumberSanity, Valid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[1]");
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_array(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestNumberSanity, Valid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[1.0]");
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_array(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestNumberSanity, Valid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer(
		"[2394309382309842309234825.62345235323253253220398443213241234"
		"123431413e90234098320982340924382340982349023423498234908234]"
		);
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_array(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestNumberSanity, Valid4)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[-50]");
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_array(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

// vim: set noet ts=4 sw=4 tw=80:
