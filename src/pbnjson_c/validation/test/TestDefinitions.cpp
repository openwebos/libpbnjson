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

#include "../parser_api.h"
#include "../uri_resolver.h"
#include "../validator.h"
#include <gtest/gtest.h>


TEST(TestDefinitions, First)
{
	auto u = uri_resolver_new();
	ASSERT_TRUE(u != NULL);
	auto v = parse_schema(
		"{"
			"\"id\": \"file://test.json\","
			"\"definitions\": {"
				"\"a\": {\"type\": \"null\"},"
				"\"b\": {\"type\": \"boolean\"},"
				"\"c\": {\"type\": \"string\"}"
			"},"
			"\"type\": \"object\","
			"\"properties\": {}"
		"}",
		u, "file://test2.json",
		NULL, NULL
		);
	ASSERT_TRUE(v != NULL);
	validator_unref(v);

	char *d = uri_resolver_dump(u);
	EXPECT_STREQ("file://test.json: # #/definitions/a #/definitions/b #/definitions/c\n", d);
	g_free(d);
	uri_resolver_free(u);
}
