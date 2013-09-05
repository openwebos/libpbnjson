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

#include "../combined_validator.h"
#include "../validation_api.h"
#include "../generic_validator.h"
#include "../null_validator.h"
#include "../boolean_validator.h"
#include <gtest/gtest.h>

using namespace std;

class TestAllOfValidator : public ::testing::Test
{
protected:
	CombinedValidator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		v = all_of_validator_new();
		s = validation_state_new(&v->base, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
		combined_validator_release(v);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestAllOfValidator *n = reinterpret_cast<TestAllOfValidator *>(ctxt);
		if (!n)
			return;
		n->error = error;
	}
};

TEST_F(TestAllOfValidator, OnlyGeneric)
{
	combined_validator_add_value(v, validator_ref(GENERIC_VALIDATOR));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestAllOfValidator, GenericAndNullPositive)
{
	combined_validator_add_value(v, validator_ref(GENERIC_VALIDATOR));
	combined_validator_add_value(v, validator_ref(NULL_VALIDATOR));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, this));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestAllOfValidator, GenericAndNullNegative)
{
	combined_validator_add_value(v, validator_ref(GENERIC_VALIDATOR));
	combined_validator_add_value(v, validator_ref(NULL_VALIDATOR));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s, this));
	//EXPECT_EQ(VEC_NOT_NULL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestAllOfValidator, AlwaysFails1)
{
	combined_validator_add_value(v, (Validator *) boolean_validator_new());
	combined_validator_add_value(v, validator_ref(NULL_VALIDATOR));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_boolean(true)), s, this));
	//EXPECT_EQ(VEC_NOT_NULL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}

TEST_F(TestAllOfValidator, AlwaysFails2)
{
	combined_validator_add_value(v, (Validator *) boolean_validator_new());
	combined_validator_add_value(v, validator_ref(NULL_VALIDATOR));
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, this));
	//EXPECT_EQ(VEC_NOT_BOOLEAN, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
}
