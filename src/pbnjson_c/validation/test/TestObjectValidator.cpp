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

#include "../object_validator.h"
#include "../null_validator.h"
#include "../number_validator.h"
#include "../string_validator.h"
#include "../validation_api.h"
#include "../generic_validator.h"
#include "../object_properties.h"
#include "../object_required.h"
#include "Util.hpp"
#include <gtest/gtest.h>

using namespace std;

class TestObjectValidator : public ::testing::Test
{
protected:
	ObjectProperties *p;
	ObjectRequired *r;
	ObjectValidator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		p = object_properties_new();
		r = object_required_new();
		v = object_validator_new();
		validator_set_object_properties(&v->base, p);
		validator_set_object_required(&v->base, r);
		s = validation_state_new(&v->base, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
		object_validator_release(v);
		object_properties_unref(p);
		object_required_unref(r);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestObjectValidator *n = reinterpret_cast<TestObjectValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

TEST_F(TestObjectValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_OBJECT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, EmptyObject)
{
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, GenericProperties)
{
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("a", 1)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, SpecificNullPropertiesPositive)
{
	object_properties_add_key(p, "null", NULL_VALIDATOR);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("null", 4)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(NULL_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, SpecificNullPropertiesNegative)
{
	object_properties_add_key(p, "null", NULL_VALIDATOR);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("null", 4)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1", 1)), s, this));
	EXPECT_EQ(VEC_NOT_NULL, error);
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, SpecificMultiplePropertiesPositive)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(number_validator_add_max_constraint((NumberValidator *)vnum.get(), "10"));

	object_properties_add_key(p, "null", NULL_VALIDATOR);
	object_properties_add_key(p, "num", validator_ref(vnum.get()));

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("num", 3)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(vnum.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_number("3", 1)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, SpecificMultiplePropertiesNevative)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(number_validator_add_max_constraint((NumberValidator *)vnum.get(), "10"));

	object_properties_add_key(p, "null", NULL_VALIDATOR);
	object_properties_add_key(p, "num", validator_ref(vnum.get()));

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("num", 3)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(vnum.get() == s->validator_stack->data);
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_EQ(VEC_NOT_NUMBER, error);
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, SpecificMultiplePropertiesNevativeOnInnerCondition)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(number_validator_add_max_constraint((NumberValidator *)vnum.get(), "10"));

	object_properties_add_key(p, "null", NULL_VALIDATOR);
	object_properties_add_key(p, "num", validator_ref(vnum.get()));

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("num", 3)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(vnum.get() == s->validator_stack->data);
	EXPECT_FALSE(validation_check(&(e = validation_event_number("12", 2)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
}

TEST_F(TestObjectValidator, OnlyEmptyObjectAllowed)
{
	validator_set_object_additional_properties(&v->base, NULL);

	EXPECT_TRUE(validate_json_plain("{}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"a\": true}", &v->base));
}

TEST_F(TestObjectValidator, DisallowedAdditionalProperties)
{
	object_properties_add_key(p, "null", NULL_VALIDATOR);
	validator_set_object_additional_properties(&v->base, NULL);

	EXPECT_TRUE(validate_json_plain("{}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"null\": null}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"other\": 5}", &v->base));
}

TEST_F(TestObjectValidator, AdditionalPropertiesDefault)
{
	object_properties_add_key(p, "null", NULL_VALIDATOR);

	EXPECT_TRUE(validate_json_plain("{\"null\": null, \"other\": \"data\"}", &v->base));
}

TEST_F(TestObjectValidator, AdditionalPropertiesSchema)
{
	object_properties_add_key(p, "any", GENERIC_VALIDATOR);
	validator_set_object_additional_properties(&v->base, NULL_VALIDATOR);

	EXPECT_TRUE(validate_json_plain("{\"any\": false}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"any\": false, \"other\": null}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"any\": false, \"other\": false}", &v->base));
}

TEST_F(TestObjectValidator, RequiredSchemaError)
{
	ASSERT_TRUE(object_required_add_key(r, "id"));

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_end()), s, this));
	EXPECT_EQ(VEC_MISSING_REQUIRED_KEY, error);
}

TEST_F(TestObjectValidator, RequiredSchema)
{
	ASSERT_TRUE(object_required_add_key(r, "id"));

	EXPECT_TRUE(validate_json_plain("{\"id\":1}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"id\":1, \"a\":null}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", &v->base));

	ASSERT_TRUE(object_required_add_key(r, "a"));
	object_properties_add_key(p, "a", NULL_VALIDATOR);

	EXPECT_FALSE(validate_json_plain("{\"id\":1}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"id\":1, \"a\":null}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"id\":1, \"a\":[]}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"a\":null}", &v->base));
}

TEST_F(TestObjectValidator, MaxPropertiesPositive)
{
	object_validator_set_max_properties(v, 1);

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("a", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
}

TEST_F(TestObjectValidator, MaxPropertiesNegative)
{
	object_validator_set_max_properties(v, 1);

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("a", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_key("b", 1)), s, this));
	EXPECT_EQ(VEC_TOO_MANY_KEYS, error);
}

TEST_F(TestObjectValidator, MinPropertiesPositive)
{
	object_validator_set_min_properties(v, 1);

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("a", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
}

TEST_F(TestObjectValidator, MinPropertiesNegative)
{
	object_validator_set_min_properties(v, 1);

	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_end()), s, this));
	EXPECT_EQ(VEC_NOT_ENOUGH_KEYS, error);
}

TEST_F(TestObjectValidator, MaxProperties)
{
	object_validator_set_max_properties(v, 2);

	EXPECT_TRUE(validate_json_plain("{\"a\":null}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"a\":null, \"b\":true}", &v->base));
	EXPECT_FALSE(validate_json_plain("{\"a\":null, \"b\":true, \"c\":[]}", &v->base));
}

TEST_F(TestObjectValidator, MinProperties)
{
	object_validator_set_min_properties(v, 2);

	EXPECT_FALSE(validate_json_plain("{\"a\":null}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"a\":null, \"b\":true}", &v->base));
	EXPECT_TRUE(validate_json_plain("{\"a\":null, \"b\":true, \"c\":[]}", &v->base));
}
