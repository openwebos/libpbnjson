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

#include <gtest/gtest.h>
#include <pbnjson.h>
#include <string>

using namespace std;

namespace {

static const string resolution_dir = string{SCHEMA_DIR} + "contact/";

class TestSchemaContact : public ::testing::Test
{
protected:
	static jschema_ref schema;
	static JSchemaResolver resolver;
	static JSchemaInfo schema_info;
	jvalue_ref parsed;

	static void SetUpTestCase()
	{
		schema = jschema_parse_file((resolution_dir + "Contact.schema").c_str(), NULL);
		ASSERT_TRUE(schema != NULL);

		resolver.m_resolve = &SimpleResolver;

		jschema_info_init(&schema_info, schema, &resolver, NULL);
		ASSERT_TRUE(jschema_resolve(&schema_info));
	}

	static void TearDownTestCase()
	{
		jschema_release(&schema);
	}

	virtual void SetUp()
	{
		parsed = NULL;
	}

	virtual void TearDown()
	{
		j_release(&parsed);
	}

	static JSchemaResolutionResult SimpleResolver(JSchemaResolverRef resolver,
	                                              jschema_ref *resolved)
	{
		string resource(resolver->m_resourceToResolve.m_str,
		                resolver->m_resourceToResolve.m_len);

		string lookup_path = resolution_dir + "/" + resource + ".schema";
		if (-1 == ::access(lookup_path.c_str(), F_OK))
			return SCHEMA_NOT_FOUND;

		*resolved = jschema_parse_file(lookup_path.c_str(), NULL);
		if (*resolved == NULL)
			return SCHEMA_INVALID;

		return SCHEMA_RESOLVED;
	}

};

jschema_ref TestSchemaContact::schema = NULL;
JSchemaResolver TestSchemaContact::resolver;
JSchemaInfo TestSchemaContact::schema_info;

} // namespace

TEST_F(TestSchemaContact, Invalid1)
{
	ASSERT_TRUE(schema != NULL);
	const raw_buffer INPUT = j_cstr_to_buffer("");

	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_null(parsed));
	EXPECT_FALSE(jis_valid(parsed));
}

TEST_F(TestSchemaContact, Valid1)
{
	ASSERT_TRUE(schema != NULL);
	const raw_buffer INPUT = j_cstr_to_buffer(
		"{"
			"\"contactIds\": [ \"1\" ],"
			"\"displayIndex\": \"first name\""
		"}"
		);
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestSchemaContact, Valid2)
{
	ASSERT_TRUE(schema != NULL);
	const raw_buffer INPUT = j_cstr_to_buffer("{}");
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestSchemaContact, Valid3)
{
	ASSERT_TRUE(schema != NULL);
	const raw_buffer INPUT = j_cstr_to_buffer(
		"{"
			"\"displayName\": \"\","
			"\"name\": {},"
			"\"birthday\": \"\","
			"\"anniversary\": \"\","
			"\"gender\": \"undisclosed\""
		"}"
		);
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL, true));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

// vim: set noet ts=4 sw=4 tw=80:
