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

#include "../array_validator.h"
#include "../array_items.h"
#include "../object_validator.h"
#include "../null_validator.h"
#include "../number_validator.h"
#include "../string_validator.h"
#include "../validation_api.h"
#include "../generic_validator.h"
#include "../error_code.h"
#include "Util.hpp"
#include <gtest/gtest.h>

using namespace std;

class TestArrayValidator : public ::testing::Test
{
protected:
	ArrayValidator *v;
	ArrayItems *items;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		items = array_items_new();
		v = array_validator_new();
		validator_set_array_items(&v->base, items);
		s = validation_state_new(&v->base, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
		array_validator_release(v);
		array_items_unref(items);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestArrayValidator *n = reinterpret_cast<TestArrayValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

TEST_F(TestArrayValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, Number)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1", 1)), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, Boolean)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, String)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_string("hello", 5)), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, ObjectStart)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_start()), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, ObjectKey)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_key("a", 1)), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, ObjectEnd)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_end()), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, ArrayEnd)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_arr_end()), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, EmptyArray)
{
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, AnyValue)
{
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// null value
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// number value
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// boolean value
	EXPECT_TRUE(validation_check(&(e = validation_event_boolean(true)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// string value
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// empty array value
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// non empty array value
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// empty object value
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	// non empty object value
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("a", 1)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, GeneralValidatorPositive)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_set_generic_item(items, validator_ref(vnum.get())));
	EXPECT_EQ(2, vnum->ref_count);
	ASSERT_EQ(vnum.get(), items->generic_validator);
	ASSERT_EQ(NULL, items->validators);

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, GeneralValidatorNegative)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_set_generic_item(items, validator_ref(vnum.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_NUMBER, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, GeneralArrayValidatorPositive)
{
	auto varr = mk_ptr((Validator *)array_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_set_generic_item(items, validator_ref(varr.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(varr.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(varr.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(varr.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, GeneralArrayValidatorNegative)
{
	auto varr = mk_ptr((Validator *)array_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_set_generic_item(items, validator_ref(varr.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(varr.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_ARRAY, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, GeneralObjectValidatorPositive)
{
	auto vobj = mk_ptr((Validator *)object_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_set_generic_item(items, validator_ref(vobj.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(vobj.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(vobj.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_key("a", 1)), s, NULL));
	EXPECT_EQ(3, g_slist_length(s->validator_stack));
	EXPECT_TRUE(GENERIC_VALIDATOR == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, GeneralObjectValidatorNegative)
{
	auto vobj = mk_ptr((Validator *)object_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_set_generic_item(items, validator_ref(vobj.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(2, g_slist_length(s->validator_stack));
	EXPECT_TRUE(vobj.get() == s->validator_stack->data);
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_OBJECT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, SpecificValidatorsLessThanNeeded)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vnum.get())));

	auto vstr = mk_ptr((Validator *)string_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vstr.get())));

	ASSERT_TRUE(v->items);
	ASSERT_EQ(2, array_items_items_length(v->items));
	EXPECT_EQ(2, vnum->ref_count);
	EXPECT_EQ(2, vstr->ref_count);
	EXPECT_EQ(vnum.get(), v->items->validators->data);
	EXPECT_EQ(vstr.get(), g_list_last(v->items->validators)->data);

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, SpecificValidatorsPerfectMatch)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vnum.get())));

	auto vstr = mk_ptr((Validator *)string_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vstr.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, SpecificValidatorsMoreThanNeeded)
{
	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vnum.get())));

	auto vstr = mk_ptr((Validator *)string_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vstr.get())));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, AdditionalItemsDisallowedPositive)
{
	ASSERT_TRUE(array_items_add_item(items, validator_ref(GENERIC_VALIDATOR)));
	validator_set_array_additional_items(&v->base, NULL);

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, AdditionalItemsDisallowedNegative)
{
	ASSERT_TRUE(array_items_add_item(items, validator_ref(GENERIC_VALIDATOR)));
	validator_set_array_additional_items(&v->base, NULL);

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_ARRAY_TOO_LONG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, AdditionalItemsPositive)
{
	auto vstr = mk_ptr((Validator *)string_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vstr.get())));

	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	validator_set_array_additional_items(&v->base, vnum.get());

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, AdditionalItemsNegative)
{
	auto vstr = mk_ptr((Validator *)string_validator_new(), validator_unref);
	ASSERT_TRUE(array_items_add_item(items, validator_ref(vstr.get())));

	auto vnum = mk_ptr((Validator *)number_validator_new(), validator_unref);
	validator_set_array_additional_items(&v->base, vnum.get());

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_NUMBER, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, OnlyEmptyArrayAllowedPositive)
{
	ASSERT_TRUE(array_items_set_zero_items(items));
	validator_set_array_additional_items(&v->base, NULL);

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, OnlyEmptyArrayAllowedNegative)
{
	ASSERT_TRUE(array_items_set_zero_items(items));
	validator_set_array_additional_items(&v->base, NULL);

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_ARRAY_TOO_LONG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, MinItemsPositive)
{
	ASSERT_TRUE(array_validator_set_min_items(v, 2));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_TRUE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, MinItemsNegative)
{
	ASSERT_TRUE(array_validator_set_min_items(v, 2));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_FALSE(validation_check(&(e = validation_event_arr_end()), s, this));
	EXPECT_EQ(VEC_ARRAY_TOO_SHORT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, MaxItemsPositive)
{
	ASSERT_TRUE(array_validator_set_max_items(v, 2));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_TRUE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestArrayValidator, MaxItemsNegative)
{
	ASSERT_TRUE(array_validator_set_max_items(v, 2));

	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_TRUE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_ARRAY_TOO_LONG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}
