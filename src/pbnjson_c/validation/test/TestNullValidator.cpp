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

#include "../null_validator.h"
#include "../validation_api.h"
#include <gtest/gtest.h>

using namespace std;

class TestNullValidator : public ::testing::Test
{
protected:
	Validator *v;
	ValidationState *s;
	ValidationEvent e;
	ValidationErrorCode error;

	virtual void SetUp()
	{
		static Notification notify { &OnError };

		v = NULL_VALIDATOR;
		s = validation_state_new(v, NULL, &notify);
		error = VEC_OK;
	}

	virtual void TearDown()
	{
		validation_state_free(s);
	}

	static void OnError(ValidationState *s, ValidationErrorCode error, void *ctxt)
	{
		TestNullValidator *n = reinterpret_cast<TestNullValidator *>(ctxt);
		if (n)
			n->error = error;
	}
};

TEST_F(TestNullValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, NULL));
}

TEST_F(TestNullValidator, Array)
{
	EXPECT_FALSE(validation_check(&(e = validation_event_arr_start()), s, this));
	EXPECT_EQ(VEC_NOT_NULL, error);
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_null()), s, NULL));
}
