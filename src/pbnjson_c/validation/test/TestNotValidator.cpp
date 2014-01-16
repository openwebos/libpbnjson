// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
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

#include "../combined_validator.h"
#include "../validation_api.h"
#include "../generic_validator.h"
#include "../null_validator.h"
#include "../boolean_validator.h"
#include "../number_validator.h"
#include "Util.hpp"
#include <gtest/gtest.h>

using namespace std;

class TestNotValidator : public ::testing::Test
{
protected:
	CombinedValidator *v;
	ValidationEvent e;
	ValidationErrorCode error;
	static Notification notify;

	virtual void SetUp()
	{
		v = not_validator_new();
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		combined_validator_release(v);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestNotValidator *n = reinterpret_cast<TestNotValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

Notification TestNotValidator::notify = { &OnError };

TEST_F(TestNotValidator, NotBoolean)
{
	combined_validator_add_value(v, boolean_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s.get(), this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNotValidator, NotBooleanAndNumberPositive)
{
	combined_validator_add_value(v, boolean_validator_instance());
	combined_validator_add_value(v, number_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s.get(), this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNotValidator, NotBooleanAndNumberNegative)
{
	combined_validator_add_value(v, boolean_validator_instance());
	combined_validator_add_value(v, number_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s.get(), this));
	EXPECT_EQ(VEC_SOME_OF_NOT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNotValidator, AlwaysFails1)
{
	combined_validator_add_value(v, generic_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s.get(), this));
	EXPECT_EQ(VEC_SOME_OF_NOT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNotValidator, AlwaysFails2)
{
	combined_validator_add_value(v, generic_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s.get(), this));
	EXPECT_EQ(VEC_SOME_OF_NOT, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNotValidator, NotNullWithObject)
{
	combined_validator_add_value(v, null_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s.get(), this));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s.get(), this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestNotValidator, NotNullWithArray)
{
	combined_validator_add_value(v, null_validator_instance());
	auto s = mk_ptr(validation_state_new(&v->base, NULL, &notify), validation_state_free);

	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s.get(), this));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s.get(), this));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s.get(), this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}
