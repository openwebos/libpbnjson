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

#include "../validation_api.h"
#include "../parser_api.h"
#include "../uri_resolver.h"
#include "Util.hpp"
#include <gtest/gtest.h>

using namespace std;

TEST(TestArrayValidator2, Reference)
{
	auto uri_resolver = mk_ptr(uri_resolver_new(), uri_resolver_free);
	char const *const SCHEMA =
		"{"
			"\"definitions\": {"
				"\"b\": { \"type\": \"boolean\" }"
			"},"
			"\"type\": \"array\","
			"\"items\": { \"$ref\": \"#/definitions/b\"}"
		"}";
	auto v = mk_ptr(parse_schema(SCHEMA, uri_resolver.get(), "", nullptr, nullptr),
	                validator_unref);
	ASSERT_TRUE(v != NULL);

	EXPECT_TRUE(validate_json("[]", v.get(), uri_resolver.get(), nullptr));
	EXPECT_TRUE(validate_json("[true, false]", v.get(), uri_resolver.get(), nullptr));
	EXPECT_FALSE(validate_json("[null]", v.get(), uri_resolver.get(), nullptr));
	EXPECT_FALSE(validate_json("[3.14, \"Pi\"]", v.get(), uri_resolver.get(), nullptr));
}
