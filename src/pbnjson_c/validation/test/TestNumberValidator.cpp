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

class TestNumberValidator : public ::testing::Test
{
protected:
	NumberValidator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		v = number_validator_new();
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
		TestNumberValidator *n = reinterpret_cast<TestNumberValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

TEST_F(TestNumberValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_NUMBER, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, Number)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithNonExclusiveMinConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2.1", 3)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithNonExclusiveMinConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithNonExclusiveMinConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1.9", 3)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithExclusiveMinConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2.1", 3)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithExclusiveMinConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	ASSERT_TRUE(number_validator_add_min_exclusive_constraint(v, true));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithExclusiveMinConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "2"));
	ASSERT_TRUE(number_validator_add_min_exclusive_constraint(v, true));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("1.9", 3)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithNonExclusiveMaxConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1.9", 3)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithNonExclusiveMaxConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("2", 1)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithNonExclusiveMaxConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2.1", 3)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithExclusiveMaxConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	ASSERT_TRUE(number_validator_add_max_exclusive_constraint(v, true));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1.9", 3)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithExclusiveMaxConstraintEquals)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	ASSERT_TRUE(number_validator_add_max_exclusive_constraint(v, true));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2", 1)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberWithExclusiveMaxConstraintNegative)
{
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	ASSERT_TRUE(number_validator_add_max_exclusive_constraint(v, true));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2.1", 3)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberMinMaxConstraintPositive)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "1"));
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_TRUE(validation_check(&(e = validation_event_number("1.5", 3)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberMinMaxConstraintNegativeLess)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "1"));
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("0.5", 3)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_SMALL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNumberValidator, NumberMinMaxConstraintNegativeGreater)
{
	ASSERT_TRUE(number_validator_add_min_constraint(v, "1"));
	ASSERT_TRUE(number_validator_add_max_constraint(v, "2"));
	EXPECT_FALSE(validation_check(&(e = validation_event_number("2.5", 3)), s, this));
	EXPECT_EQ(VEC_NUMBER_TOO_BIG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}
