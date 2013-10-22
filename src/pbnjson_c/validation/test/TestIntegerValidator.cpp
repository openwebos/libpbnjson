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

#include "../number_validator.h"
#include "../validation_api.h"
#include <gtest/gtest.h>

using namespace std;

class TestIntegerValidator : public ::testing::Test
{
protected:
	NumberValidator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		v = integer_validator_new();
		s = validation_state_new(&v->base, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
		number_validator_release(v);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestIntegerValidator *n = reinterpret_cast<TestIntegerValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

TEST_F(TestIntegerValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_NUMBER, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, Number)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithDecimalPoint)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2.1", 3)), s, this));
	EXPECT_EQ(VEC_NOT_INTEGER_NUMBER, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExpPart)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2e-2", 4)), s, this));
	EXPECT_EQ(VEC_NOT_INTEGER_NUMBER, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithNonExclusiveMinConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("3", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithNonExclusiveMinConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithNonExclusiveMinConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExclusiveMinConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("3", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExclusiveMinConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	number_validator_add_min_exclusive_constraint(v, true);
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExclusiveMinConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	number_validator_add_min_exclusive_constraint(v, true);
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithNonExclusiveMaxConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithNonExclusiveMaxConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithNonExclusiveMaxConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("3", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExclusiveMaxConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	number_validator_add_max_exclusive_constraint(v, true);
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExclusiveMaxConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	number_validator_add_max_exclusive_constraint(v, true);
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberWithExclusiveMaxConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	number_validator_add_max_exclusive_constraint(v, true);
	EXPECT_FALSE(validation_check(&(e = validation_event_number("3", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberMinMaxConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "1"));
	ASSERT_TRUE(number_validator_add_max_constraint(v, "3"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberMinMaxConstraintNegativeLess)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "1"));
	ASSERT_TRUE(number_validator_add_max_constraint(v, "3"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("0", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestIntegerValidator, NumberMinMaxConstraintNegativeGreater)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "1"));
	ASSERT_TRUE(number_validator_add_max_constraint(v, "3"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("4", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}
