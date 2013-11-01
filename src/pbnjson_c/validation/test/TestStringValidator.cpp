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

#include "../string_validator.h"
#include "../validation_api.h"
#include "../parser_context.h"
#include <gtest/gtest.h>

using namespace std;

class TestStringValidator : public ::testing::Test
{
protected:
	StringValidator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		v = string_validator_new();
		s = validation_state_new(&v->base, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
		string_validator_release(v);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestStringValidator *n = reinterpret_cast<TestStringValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};


TEST_F(TestStringValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(VEC_NOT_STRING, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, String)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMinLengthPositive)
{
	string_validator_add_min_length_constraint(v, 3);
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMinLengthPositiveEdge)
{
	string_validator_add_min_length_constraint(v, 5);
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMinLengthNegative)
{
	string_validator_add_min_length_constraint(v, 6);
	EXPECT_FALSE(validation_check(&(e = validation_event_string("hello", 5)), s, this));
	EXPECT_EQ(VEC_STRING_TOO_SHORT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMaxLengthPositive)
{
	string_validator_add_max_length_constraint(v, 8);
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMaxLengthPositiveEdge)
{
	string_validator_add_max_length_constraint(v, 5);
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMaxLengthNegative)
{
	string_validator_add_max_length_constraint(v, 4);
	EXPECT_FALSE(validation_check(&(e = validation_event_string("hello", 5)), s, this));
	EXPECT_EQ(VEC_STRING_TOO_LONG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMinMaxLengthPositive)
{
	string_validator_add_max_length_constraint(v, 8);
	string_validator_add_min_length_constraint(v, 3);
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello", 5)), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMinMaxLengthNegativeLess)
{
	string_validator_add_max_length_constraint(v, 8);
	string_validator_add_min_length_constraint(v, 3);
	EXPECT_FALSE(validation_check(&(e = validation_event_string("h", 1)), s, this));
	EXPECT_EQ(VEC_STRING_TOO_SHORT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, StringWithMinMaxLengthNegativeGreater)
{
	string_validator_add_max_length_constraint(v, 8);
	string_validator_add_min_length_constraint(v, 3);
	EXPECT_FALSE(validation_check(&(e = validation_event_string("hello world", 11)), s, this));
	EXPECT_EQ(VEC_STRING_TOO_LONG, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestStringValidator, ExpectedValue)
{
	StringSpan expected_value = { "hello world", 11 };
	string_validator_add_expected_value(v, &expected_value);
	EXPECT_TRUE(validation_check(&(e = validation_event_string("hello world", 11)), s, this));
}

TEST_F(TestStringValidator, ExpectedValueNegative)
{
	StringSpan expected_value = { "hello world", 11 };
	string_validator_add_expected_value(v, &expected_value);
	EXPECT_FALSE(validation_check(&(e = validation_event_string("hello", 5)), s, this));
}
