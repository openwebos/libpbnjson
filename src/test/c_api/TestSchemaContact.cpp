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
#include <uriparser/Uri.h>

using namespace std;

namespace {

static const string resolution_dir = string{SCHEMA_DIR} + "contact/";
static const string localref_dir = string{SCHEMA_DIR} + "localref/";
static const string xref_dir = string{SCHEMA_DIR} + "xref/";

class TestSchemaContact : public ::testing::Test
{
protected:
	static jschema_ref schema;
	static JSchemaResolver resolver;
	static JSchemaResolver xResolver;
	static JSchemaInfo schema_info;
	jvalue_ref parsed;

	static void SetUpTestCase()
	{
		resolver.m_resolve = &SimpleResolver;
		schema = jschema_parse_file_resolve((resolution_dir + "Contact.schema").c_str(),
		                                    (resolution_dir + "Contact.schema").c_str(), NULL, &resolver);
		ASSERT_TRUE(schema != NULL);


		jschema_info_init(&schema_info, schema, &resolver, NULL);
		ASSERT_TRUE(jschema_resolve_ex(schema, &resolver));
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

	static JSchemaResolutionResult XResolver(JSchemaResolverRef resolver,
	                                         jschema_ref *resolved)
	{
		string resource(resolver->m_resourceToResolve.m_str,
		                resolver->m_resourceToResolve.m_len);
		char unixUri[1024];
		uriUriStringToUnixFilenameA(resource.c_str(), unixUri);

		if (-1 == ::access(unixUri, F_OK)) {
			std::cerr << "SCHEMA_NOT_FOUND " << unixUri << std::endl;
			return SCHEMA_NOT_FOUND;
		}

		*resolved = jschema_parse_file_resolve(unixUri, NULL, NULL, resolver);
		if (*resolved == NULL) {
			std::cerr << "SCHEMA_INVALID " << resource.c_str() << std::endl;
			return SCHEMA_INVALID;
		}

		return SCHEMA_RESOLVED;
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
JSchemaResolver TestSchemaContact::xResolver;
JSchemaInfo TestSchemaContact::schema_info;

} // namespace

TEST_F(TestSchemaContact, Invalid1)
{
	ASSERT_TRUE(schema != NULL);
	const raw_buffer INPUT = j_cstr_to_buffer("");

	EXPECT_FALSE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL));
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
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestSchemaContact, Valid2)
{
	ASSERT_TRUE(schema != NULL);
	const raw_buffer INPUT = j_cstr_to_buffer("{}");
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL));
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
	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &schema_info, NULL));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &schema_info);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_check_schema(parsed, &schema_info));
}

TEST_F(TestSchemaContact, localReferences)
{
	xResolver.m_resolve = &TestSchemaContact::XResolver;
	jschema_ref xschema = jschema_parse_file_resolve((localref_dir + "rA.schema").c_str(),
	                                                 NULL, NULL, &xResolver);
	ASSERT_TRUE(xschema != NULL);

	JSchemaInfo sinfo;
	jschema_info_init(&sinfo, xschema, NULL, NULL);

	const raw_buffer INPUT = j_cstr_to_buffer( R"schema(
		{
			"name": "Alisha",
			"flag": true,
			"field": {
				"familyName": "Lalala",
				"flagB": true
			}
		}
	)schema");

	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &sinfo, NULL));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &sinfo);
	EXPECT_TRUE(jis_object(parsed));

	const raw_buffer FAIL_INPUT = j_cstr_to_buffer( R"schema(
		{
			"name": "Alisha",
			"flag": true,
			"field": {
				"familyName": false,
				"flagB": true
			}
		}
	)schema");

	EXPECT_FALSE(jsax_parse_ex(NULL, FAIL_INPUT, &sinfo, NULL));

	jschema_release(&xschema);
}

TEST_F(TestSchemaContact, crossReferences)
{
	memset(&xResolver, 0, sizeof(xResolver));
	xResolver.m_resolve = &TestSchemaContact::XResolver;
	jschema_ref xschema = jschema_parse_file_resolve((xref_dir + "xA.schema").c_str(),
	                                                 NULL, NULL, &xResolver);
	ASSERT_TRUE(xschema != NULL);

	JSchemaInfo sinfo;
	jschema_info_init(&sinfo, xschema, NULL, NULL);

	const raw_buffer INPUT = j_cstr_to_buffer( R"schema(
		{
			"name": "Alisha",
			"flag": true,
			"field": {
				"familyName": "Simpson",
				"fieldA": {
					"name": "Andrii",
					"flag": false
				},
				"fieldC": {
					"stringC": "Hi",
					"fieldB": {
						"familyName": "Griffin"
					}
				}
			}
		}
	)schema");

	EXPECT_TRUE(jsax_parse_ex(NULL, INPUT, &sinfo, NULL));
	parsed = jdom_parse(INPUT, DOMOPT_NOOPT, &sinfo);
	EXPECT_TRUE(jis_object(parsed));

	const raw_buffer FAIL_INPUT = j_cstr_to_buffer( R"schema(
		{
			"name": "Alisha",
			"flag": true,
			"field": {
				"familyName": "Simpson",
				"fieldA": {
					"name": "Andrii",
					"flag": "O NO STRING!"
				}
			}
		}
	)schema");

	EXPECT_FALSE(jsax_parse_ex(NULL, FAIL_INPUT, &sinfo, NULL));

	jschema_release(&xschema);
}

// vim: set noet ts=4 sw=4 tw=80:
