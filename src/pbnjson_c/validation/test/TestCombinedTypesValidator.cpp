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

#include "../combined_types_validator.h"
#include "../null_validator.h"
#include "../number_validator.h"
#include "../string_validator.h"
#include "../validation_api.h"
#include "Util.hpp"
#include <gtest/gtest.h>

using namespace std;

class TestCombinedTypesValidator : public ::testing::Test
{
protected:
	CombinedTypesValidator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		v = combined_types_validator_new();
		s = validation_state_new(&v->base, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
		combined_types_validator_release(v);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestCombinedTypesValidator *n = reinterpret_cast<TestCombinedTypesValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

TEST_F(TestCombinedTypesValidator, EmptyNull)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_TYPE_NOT_ALLOWED, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestCombinedTypesValidator, EmptyNumber)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1", 1)), s, this));
	EXPECT_EQ(VEC_TYPE_NOT_ALLOWED, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestCombinedTypesValidator, EmptyBoolean)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_EQ(VEC_TYPE_NOT_ALLOWED, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestCombinedTypesValidator, OnlyNullPositive)
{
	combined_types_validator_set_type(v, "null", 4);
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestCombinedTypesValidator, OnlyNullNegativeOnBool)
{
	combined_types_validator_set_type(v, "null", 4);
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s, this));
	EXPECT_EQ(VEC_TYPE_NOT_ALLOWED, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestCombinedTypesValidator, OnlyNullNegativeOnString)
{
	combined_types_validator_set_type(v, "null", 4);
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_string("a", 1)), s, this));
	EXPECT_EQ(VEC_TYPE_NOT_ALLOWED, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

class TestCombinedTypesValidatorOnlyNumAndString : public TestCombinedTypesValidator
{
protected:
	virtual void SetUp()
	{
		TestCombinedTypesValidator::SetUp();
		combined_types_validator_set_type(v, "number", 6);
		combined_types_validator_set_type(v, "string", 6);
	}
};

TEST_F(TestCombinedTypesValidatorOnlyNumAndString, PositiveOnString)
{
	EXPECT_TRUE(validate_json_plain("\"hello\"", &v->base));
}

TEST_F(TestCombinedTypesValidatorOnlyNumAndString, PositiveOnNum)
{
	EXPECT_TRUE(validate_json_plain("1.2", &v->base));
}

TEST_F(TestCombinedTypesValidatorOnlyNumAndString, NegativeOnNull)
{
	EXPECT_FALSE(validate_json_plain("null", &v->base));
}

TEST_F(TestCombinedTypesValidatorOnlyNumAndString, NegativeOnObject)
{
	EXPECT_FALSE(validate_json_plain("{}", &v->base));
}

TEST_F(TestCombinedTypesValidatorOnlyNumAndString, NegativeOnArray)
{
	EXPECT_FALSE(validate_json_plain("[]", &v->base));
}
