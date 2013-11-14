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

#include <gtest/gtest.h>
#include <pbnjson.h>
#include <string>
#include <memory>

#include "../../pbnjson_c/jschema_types_internal.h"
#include "../../pbnjson_c/validation/error_code.h"

using namespace std;

unique_ptr<jschema, function<void(jschema_ref &)>> mk_schema_ptr(jschema_ref s)
{
	unique_ptr<jschema, function<void(jschema_ref &)>>
		js { s, [](jschema_ref &s) { jschema_release(&s); } };
	return js;
}

unique_ptr<jvalue, function<void(jvalue_ref &)>> mk_jvalue_ptr(jvalue_ref v)
{
	unique_ptr<jvalue, function<void(jvalue_ref &)>>
		jv { v, [](jvalue_ref &v) { j_release(&v); } };
	return jv;
}

class TestSchemaValidationErrorReporting : public ::testing::Test
{
protected:
	JErrorCallbacks errors;
	unsigned int errorCounter;
	int errorCode;

	TestSchemaValidationErrorReporting()
	{
		errors.m_parser = &OnError;
		errors.m_schema = &OnValidationError;
		errors.m_unknown = &OnError;
		errors.m_ctxt = this;
	}

	virtual void SetUp()
	{
		errorCounter = 0;
		errorCode = VEC_OK;
	}

	static bool OnError(void *ctxt, JSAXContextRef parseCtxt)
	{
		return false;
	}

	static bool OnValidationError(void *ctxt, JSAXContextRef parseCtxt)
	{
		TestSchemaValidationErrorReporting *t = reinterpret_cast<TestSchemaValidationErrorReporting *>(ctxt);
		assert(t);
		t->errorCounter++;
		t->errorCode = parseCtxt->m_error_code;
		return false;
	}

	bool TestError(const char *schemaStr, const char *json, ValidationErrorCode error)
	{
		SetUp();
		auto schema = mk_schema_ptr(jschema_parse(j_cstr_to_buffer(schemaStr), JSCHEMA_DOM_NOOPT, NULL));
		if (!schema.get())
			return false;

		JSchemaInfo schemaInfo;
		jschema_info_init(&schemaInfo, schema.get(), NULL, &errors);

		EXPECT_TRUE(jis_null(mk_jvalue_ptr(jdom_parse(j_cstr_to_buffer(json), DOMOPT_NOOPT, &schemaInfo)).get()));
		EXPECT_EQ(error, errorCode);
		EXPECT_EQ(1, errorCounter);
		return errorCounter == 1;
	}
};

TEST_F(TestSchemaValidationErrorReporting, Null)
{
	EXPECT_TRUE(TestError("{\"type\": \"null\"}", "1", VEC_NOT_NULL));
}

TEST_F(TestSchemaValidationErrorReporting, Number)
{
	const char *schema = "{\"type\": \"number\", \"minimum\": 1, \"maximum\": 10 }";
	EXPECT_TRUE(TestError(schema, "null", VEC_NOT_NUMBER));
	EXPECT_TRUE(TestError(schema, "0", VEC_NUMBER_TOO_SMALL));
	EXPECT_TRUE(TestError(schema, "100", VEC_NUMBER_TOO_BIG));
	EXPECT_TRUE(TestError("{\"type\": \"integer\"}", "1.2", VEC_NOT_INTEGER_NUMBER));
}

TEST_F(TestSchemaValidationErrorReporting, Boolean)
{
	EXPECT_TRUE(TestError("{\"type\": \"boolean\"}", "null", VEC_NOT_BOOLEAN));
}

TEST_F(TestSchemaValidationErrorReporting, String)
{
	const char *schema = "{\"type\": \"string\", \"minLength\": 3, \"maxLength\": 10 }";
	EXPECT_TRUE(TestError(schema, "null", VEC_NOT_STRING));
	EXPECT_TRUE(TestError(schema, "\"h\"", VEC_STRING_TOO_SHORT));
	EXPECT_TRUE(TestError(schema, "\"hello world\"", VEC_STRING_TOO_LONG));
}

TEST_F(TestSchemaValidationErrorReporting, Array)
{
	const char *schema = "{\"type\": \"array\", \"minItems\": 1, \"maxItems\": 3, \"uniqueItems\": true }";
	EXPECT_TRUE(TestError(schema, "null", VEC_NOT_ARRAY));
	EXPECT_TRUE(TestError(schema, "[]", VEC_ARRAY_TOO_SHORT));
	EXPECT_TRUE(TestError(schema, "[1, 2, 3, 4]", VEC_ARRAY_TOO_LONG));
	EXPECT_TRUE(TestError(schema, "[1, 1]", VEC_ARRAY_HAS_DUPLICATES));
	EXPECT_TRUE(TestError("{\"type\": \"array\", \"items\": [{}], \"additionalItems\": false }", "[1, 1]", VEC_ARRAY_TOO_LONG));
}

TEST_F(TestSchemaValidationErrorReporting, Object)
{
	const char *schema = "{\"type\": \"object\", \"minProperties\": 1, \"maxProperties\": 2}";
	EXPECT_TRUE(TestError(schema, "null", VEC_NOT_OBJECT));
	EXPECT_TRUE(TestError(schema, "{}", VEC_NOT_ENOUGH_KEYS));
	EXPECT_TRUE(TestError(schema, "{\"a\": 1, \"b\": 2, \"c\": 3 }", VEC_TOO_MANY_KEYS));
	EXPECT_TRUE(TestError("{\"type\": \"object\", \"required\": [\"a\", \"b\"] }", "{\"a\": 1 }", VEC_MISSING_REQUIRED_KEY));
	EXPECT_TRUE(TestError("{\"type\": \"object\", \"properties\": {\"a\": {} }, \"additionalProperties\": false }", "{\"a\": 1, \"b\": 2 }", VEC_OBJECT_PROPERTY_NOT_ALLOWED));
}

TEST_F(TestSchemaValidationErrorReporting, Types)
{
	EXPECT_TRUE(TestError("{\"type\": [\"object\", \"array\"] }", "null", VEC_TYPE_NOT_ALLOWED));
}

TEST_F(TestSchemaValidationErrorReporting, Enum)
{
	EXPECT_TRUE(TestError("{\"enum\": [1, false] }", "0", VEC_UNEXPECTED_VALUE));
}

TEST_F(TestSchemaValidationErrorReporting, AllOf)
{
	EXPECT_TRUE(TestError("{\"allOf\": [{}, {\"type\": \"string\"} ] }", "0", VEC_NOT_EVERY_ALL_OF));
}

TEST_F(TestSchemaValidationErrorReporting, AnyOf)
{
	EXPECT_TRUE(TestError("{\"anyOf\": [{\"type\": \"array\"}, {\"type\": \"string\"} ] }", "0", VEC_NEITHER_OF_ANY));
}

TEST_F(TestSchemaValidationErrorReporting, OneOf)
{
	const char *schema = "{\"oneOf\": [{\"enum\": [\"hello\"]}, {\"type\": \"string\"} ] }";
	EXPECT_TRUE(TestError(schema, "\"hello\"", VEC_MORE_THAN_ONE_OF));
	EXPECT_TRUE(TestError(schema, "null", VEC_NEITHER_OF_ANY));
}

TEST_F(TestSchemaValidationErrorReporting, Complex)
{
	EXPECT_TRUE(TestError("{\"type\": \"string\", \"enum\": [\"hello\"], \"anyOf\": [{}] }", "\"h\"", VEC_UNEXPECTED_VALUE));
	EXPECT_TRUE(TestError("{\"type\": \"array\", \"anyOf\": [{}], \"uniqueItems\": true }", "[1, 1]", VEC_ARRAY_HAS_DUPLICATES));
}
