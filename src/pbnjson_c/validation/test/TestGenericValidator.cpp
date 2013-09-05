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

#include "../generic_validator.h"
#include "../validation_api.h"
#include <gtest/gtest.h>

using namespace std;

class TestGenericValidator : public ::testing::Test
{
protected:
	Validator *v;
	ValidationState *s;
	ValidationEvent e;

	virtual void SetUp()
	{
		v = GENERIC_VALIDATOR;
		s = validation_state_new(v, NULL, NULL);
	}

	virtual void TearDown()
	{
		validation_state_free(s);
	}
};

TEST_F(TestGenericValidator, Null)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&e, s, NULL));
}

TEST_F(TestGenericValidator, Object)
{
	ASSERT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_start()), s, NULL));
	EXPECT_EQ(1, g_slist_length(s->validator_stack));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_start()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_arr_end()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_null()), s, NULL));
	EXPECT_TRUE(validation_check(&(e = validation_event_obj_end()), s, NULL));
	EXPECT_EQ(0, g_slist_length(s->validator_stack));
	EXPECT_FALSE(validation_check(&(e = validation_event_obj_start()), s, NULL));
}
