/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2014 LG Electronics, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * LICENSE@@@
 ****************************************************************/

/**
 *  @file TestSchemaFromJvalue.cpp
 */

#include <memory>

#include <gtest/gtest.h>

#include "pbnjson.h"
#include "../../pbnjson_c/validation/validator.h"

using namespace std;

namespace {

	template <typename T, typename D>
	unique_ptr<T, D> mk_ptr(T *p, D d)
	{
		return std::unique_ptr<T, D>(p, d);
	}

	auto schema_str = [](const char *schemaStr) {
		return mk_ptr(
			jschema_parse(j_cstr_to_buffer(schemaStr), JSCHEMA_DOM_NOOPT, nullptr),
			[](jschema_ref x) { jschema_release(&x); }
		);
	};

	auto jvalue_str = [](const char *jsonStr, jschema_ref schema = jschema_all()) {
		JSchemaInfo schemaInfo;
		jschema_info_init(&schemaInfo, schema, NULL, NULL);

		return mk_ptr(
			jdom_parse(j_cstr_to_buffer(jsonStr), JFileOptNoOpt, &schemaInfo),
			[](jvalue_ref x) { j_release(&x); }
		);
	};

	auto schema_jvalue = [](jvalue_ref value) {
		return mk_ptr(
			jschema_parse_jvalue(value, nullptr, ""),
			[](jschema_ref x) { jschema_release(&x); }
		);
	};

} // anonymous namespace

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::Combine;

class SchemaFromJvalue : public TestWithParam<tr1::tuple<const char *, const char *>>
{ };

TEST_P(SchemaFromJvalue, Basic)
{
	auto param = GetParam();
	static_assert( 2 == tr1::tuple_size<decltype(param)>::value, "param should be a tuple of 2 values" );
	auto schema = get<0>(param);
	auto json = get<1>(param);

	auto schema_s = schema_str(schema);
	ASSERT_TRUE( !!schema_s );

	auto parsed_schema = jvalue_str(schema);
	ASSERT_TRUE( !!parsed_schema );

	auto schema_j = schema_jvalue(parsed_schema.get());
	ASSERT_TRUE( !!schema_j );

	auto value_s = jvalue_str(json, schema_s.get());
	auto value_j = jvalue_str(json, schema_j.get());

	// note that jerror() have same representation with null, so we'll check it
	// separately
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value_j.get()) );

	// we do a special comparison for null values
	if (jis_null(value_s.get()) || jis_null(value_j.get()))
	{
		EXPECT_EQ( jis_null(value_s.get()), jis_null(value_j.get()) );
	}
	else // fill for non-null values only
	{
		// we relay on the fact that different results have different string
		// representations using strings gives nicier output
		const char *str_s = jvalue_tostring(value_s.get(), jschema_all());
		const char *str_j = jvalue_tostring(value_j.get(), jschema_all());

		EXPECT_STREQ( str_s, str_j );
	}
}

// cartesian product of schemas vs json values
INSTANTIATE_TEST_CASE_P(Samples, SchemaFromJvalue,
	Combine(
		Values(
			"{}",
			"{\"type\":\"integer\"}",
			"{\"type\":\"array\", \"uniqueItems\":true}",
			"{\"properties\":{\"foo\":{\"default\":false}}}",
			"{\"properties\":{\"foo\":{\"type\":\"integer\"}}}",
			"{\"additionalProperties\":{\"type\":\"integer\"}}"
		),
		Values(
			"1",
			"true",
			"null",
			"[1,2,3]",
			"[1,2,3,1]",
			"[null,true]",
			"{}",
			"{\"foo\":1}",
			"{\"foo\":true}",
			"{\"foo\":true, \"bar\":40}",
			"{\"bar\":40}"
		)
	)
);
