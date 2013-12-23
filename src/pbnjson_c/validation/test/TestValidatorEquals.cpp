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

#include "../validator.h"
#include "../parser_api.h"
#include "../parser_context.h"
#include "../number_validator.h"
#include "../string_validator.h"
#include "../array_validator.h"
#include "../array_items.h"
#include "../object_validator.h"
#include "../object_properties.h"
#include "../object_required.h"
#include "../generic_validator.h"
#include "../null_validator.h"
#include "Util.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace std;

bool schema_equals(const char *schema1, const char *schema2)
{
	auto s1 = mk_ptr(parse_schema_bare(schema1), validator_unref);
	auto s2 = mk_ptr(parse_schema_bare(schema2), validator_unref);
	return validator_equals(s1.get(), s2.get());
}

TEST(ValidatorEquals, Basic)
{
	EXPECT_TRUE(schema_equals("{}", "{}"));
	EXPECT_FALSE(schema_equals("{}", "{\"type\":\"null\"}"));
}

TEST(ValidatorEquals, SimpleTypes)
{
	EXPECT_TRUE(schema_equals("{\"type\":\"null\"}", "{\"type\":\"null\"}"));

	EXPECT_TRUE(schema_equals("{\"type\":\"number\"}", "{\"type\":\"number\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"number\"}", "{\"type\":\"null\"}"));

	EXPECT_TRUE(schema_equals("{\"type\":\"integer\"}", "{\"type\":\"integer\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"integer\"}", "{\"type\":\"null\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"integer\"}", "{\"type\":\"number\"}"));

	EXPECT_TRUE(schema_equals("{\"type\":\"boolean\"}", "{\"type\":\"boolean\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"boolean\"}", "{\"type\":\"null\"}"));

	EXPECT_TRUE(schema_equals("{\"type\":\"string\"}", "{\"type\":\"string\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"string\"}", "{\"type\":\"null\"}"));

	EXPECT_TRUE(schema_equals("{\"type\":\"array\"}", "{\"type\":\"array\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"array\"}", "{\"type\":\"null\"}"));

	EXPECT_TRUE(schema_equals("{\"type\":\"object\"}", "{\"type\":\"object\"}"));
	EXPECT_FALSE(schema_equals("{\"type\":\"object\"}", "{\"type\":\"null\"}"));
}

TEST(ValidatorEquals, CombinedTypes)
{
	EXPECT_TRUE(schema_equals("{\"type\":[\"null\"]}", "{\"type\":[\"null\"]}"));
	EXPECT_FALSE(schema_equals("{\"type\":[\"number\"]}", "{\"type\":[\"null\"]}"));
	EXPECT_FALSE(schema_equals("{\"type\":[\"number\"]}", "{\"type\":[\"integer\"]}"));
	EXPECT_FALSE(schema_equals("{\"type\":[\"null\", \"number\"]}", "{\"type\":[\"null\", \"integer\"]}"));
	EXPECT_FALSE(schema_equals("{\"type\":[\"number\", \"string\"]}", "{\"type\":[\"number\"]}"));
	EXPECT_TRUE(schema_equals("{\"type\":[\"number\", \"string\"]}", "{\"type\":[\"string\", \"number\"]}"));
	EXPECT_TRUE(schema_equals("{\"type\":[\"number\", \"integer\"]}", "{\"type\":[\"integer\", \"number\"]}"));
}

TEST(ValidatorEquals, Number)
{
	auto v1 = mk_ptr((Validator *) number_validator_new(), validator_unref);
	auto v2 = mk_ptr((Validator *) number_validator_new(), validator_unref);

	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	StringSpan num = { "0.21e-2", 7 };
	number_validator_add_expected_value((NumberValidator *) v1.get(), &num);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	number_validator_add_expected_value((NumberValidator *) v2.get(), &num);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	Number max;
	number_init(&max);
	number_set(&max, "100");
	validator_set_number_maximum(v1.get(), &max);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_number_maximum(v2.get(), &max);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));
	number_clear(&max);

	validator_set_number_maximum_exclusive(v1.get(), true);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_number_maximum_exclusive(v2.get(), true);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));
}

TEST(ValidatorEquals, String)
{
	auto v1 = mk_ptr((Validator *) string_validator_new(), validator_unref);
	auto v2 = mk_ptr((Validator *) string_validator_new(), validator_unref);

	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	StringSpan str = { "hello", 5 };
	string_validator_add_expected_value((StringValidator *) v1.get(), &str);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	string_validator_add_expected_value((StringValidator *) v2.get(), &str);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_string_max_length(v1.get(), 10);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_string_max_length(v2.get(), 10);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_string_min_length(v1.get(), 3);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_string_min_length(v2.get(), 3);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));
}

TEST(ValidatorEquals, Array)
{
	auto v1 = mk_ptr((Validator *) array_validator_new(), validator_unref);
	auto v2 = mk_ptr((Validator *) array_validator_new(), validator_unref);

	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	auto i1 = mk_ptr(array_items_new(), array_items_unref);
	array_items_add_item(i1.get(), generic_validator_instance());
	auto i2 = mk_ptr(array_items_new(), array_items_unref);
	array_items_add_item(i2.get(), generic_validator_instance());

	validator_set_array_items(v1.get(), i1.get());
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_array_items(v2.get(), i2.get());
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_array_additional_items(v1.get(), null_validator_instance());
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_array_additional_items(v2.get(), null_validator_instance());
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_array_min_items(v1.get(), 1);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_array_min_items(v2.get(), 1);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_array_max_items(v1.get(), 10);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_array_max_items(v2.get(), 10);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_array_unique_items(v1.get(), true);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_array_unique_items(v2.get(), true);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));
}

TEST(ValidatorEquals, Object)
{
	auto v1 = mk_ptr((Validator *) object_validator_new(), validator_unref);
	auto v2 = mk_ptr((Validator *) object_validator_new(), validator_unref);

	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	auto p1 = mk_ptr(object_properties_new(), object_properties_unref);
	object_properties_add_key(p1.get(), "a", generic_validator_instance());
	auto p2 = mk_ptr(object_properties_new(), object_properties_unref);
	object_properties_add_key(p2.get(), "a", generic_validator_instance());

	validator_set_object_properties(v1.get(), p1.get());
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_object_properties(v2.get(), p2.get());
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_object_additional_properties(v1.get(), null_validator_instance());
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_object_additional_properties(v2.get(), null_validator_instance());
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	auto r1 = mk_ptr(object_required_new(), object_required_unref);
	object_required_add_key(r1.get(), "b");
	auto r2 = mk_ptr(object_required_new(), object_required_unref);
	object_required_add_key(r2.get(), "b");

	validator_set_object_required(v1.get(), r1.get());
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_object_required(v2.get(), r2.get());
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_object_max_properties(v1.get(), 10);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_object_max_properties(v2.get(), 10);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));

	validator_set_object_min_properties(v1.get(), 10);
	EXPECT_FALSE(validator_equals(v1.get(), v2.get()));
	validator_set_object_min_properties(v2.get(), 10);
	EXPECT_TRUE(validator_equals(v1.get(), v2.get()));
}

