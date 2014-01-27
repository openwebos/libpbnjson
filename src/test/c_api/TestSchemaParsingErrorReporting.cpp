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
#include <memory>

#include "../../pbnjson_c/validation/error_code.h"
#include "../../pbnjson_c/jschema_types_internal.h"

using namespace std;

unique_ptr<jschema, function<void(jschema_ref &)>> mk_ptr(jschema_ref s)
{
	unique_ptr<jschema, function<void(jschema_ref &)>>
		js { s, [](jschema_ref &s) { jschema_release(&s); } };
	return js;
}

class TestSchemaParsingErrorReporting : public ::testing::Test
{
protected:
	JErrorCallbacks error;
	int errorCounter;
	int errorCode;
	string errorMsg;

	TestSchemaParsingErrorReporting()
	{
		error.m_parser = &OnError;
		error.m_ctxt = this;
	}

	virtual void SetUp()
	{
		errorCounter = 0;
		errorMsg = "";
	}

	static bool OnError(void *ctxt, JSAXContextRef parseCtxt)
	{
		TestSchemaParsingErrorReporting *t = reinterpret_cast<TestSchemaParsingErrorReporting *>(ctxt);
		assert(t);
		t->errorCounter++;
		t->errorCode = parseCtxt->m_error_code;
		t->errorMsg = parseCtxt->errorDescription;
		return false;
	}

	bool TestError(const char *schemaStr, int errorCode)
	{
		SetUp();
		auto schema = mk_ptr(jschema_parse(j_cstr_to_buffer(schemaStr), JSCHEMA_DOM_NOOPT, &error));
		EXPECT_EQ(1, this->errorCounter);
		bool right_error = this->errorCode == errorCode;
		EXPECT_TRUE(right_error);
		// Syntax error could provide YAJL error description message
		if (errorCode != SEC_SYNTAX)
			EXPECT_EQ(this->errorMsg, SchemaGetErrorMessage(errorCode));
		return this->errorCounter == 1 && right_error;
	}
};

TEST_F(TestSchemaParsingErrorReporting, InvalidJson)
{
	EXPECT_TRUE(TestError("]", SEC_SYNTAX));
	EXPECT_TRUE(TestError("{]", SEC_SYNTAX));
	EXPECT_TRUE(TestError("[qwe]", SEC_SYNTAX));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidSchemaType1)
{
	EXPECT_TRUE(TestError("{ \"type\" : null }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : 0 }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : false }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : {} }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : {\"a\":null} }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [] }", SEC_TYPE_ARRAY_EMPTY));
	EXPECT_TRUE(TestError("{ \"type\" : \"invalid_type\" }", SEC_TYPE_VALUE));
	EXPECT_TRUE(TestError("{ \"type\" : [null] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [0] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [false] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [{}] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [{\"a\":null}] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [[]] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [[1]] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"invalid_type\"] }", SEC_TYPE_VALUE));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", null] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", 0 ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", false ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", {} ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", {\"a\":null} ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", [] ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", [1] ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", \"invalid_type\"] }", SEC_TYPE_VALUE));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", \"array\"] }", SEC_TYPE_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"type\" : [\"integer\", \"number\", \"integer\"] }", SEC_TYPE_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"type\" : [\"number\", \"integer\", \"number\"] }", SEC_TYPE_ARRAY_DUPLICATES));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMultipleOf)
{
	EXPECT_TRUE(TestError("{ \"multipleOf\" : null }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"multipleOf\" : false }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"multipleOf\" : \"mul\" }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"multipleOf\" : {} }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"multipleOf\" : [] }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"multipleOf\" : -1.2 }", SEC_MULTIPLE_OF_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"multipleOf\" : 0 }", SEC_MULTIPLE_OF_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaximum)
{
	EXPECT_TRUE(TestError("{ \"maximum\" : null }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"maximum\" : false }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"maximum\" : \"max\" }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"maximum\" : {} }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"maximum\" : [] }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : null }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : 0 }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : \"exclusive\" }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : {} }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : [] }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	// FIXME: if "exclusiveMaximum" is present "maximum" should be present also
	//EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : false }", SEC_SYNTAX));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinimum)
{
	EXPECT_TRUE(TestError("{ \"minimum\" : null }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"minimum\" : false }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"minimum\" : \"min\" }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"minimum\" : {} }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"minimum\" : [] }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : null }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : 0 }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : \"exclusive\" }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : {} }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : [] }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	// FIXME: if "exclusiveMinimum" is present "minimum" should be present also
	//EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : false }", SEC_SYNTAX));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaxLength)
{
	EXPECT_TRUE(TestError("{ \"maxLength\" : null }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : false }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : \"length\" }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : {} }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : [] }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : 1.2 }", SEC_MAX_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : 12e-2 }", SEC_MAX_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxLength\" : -1 }", SEC_MAX_LENGTH_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinLength)
{
	EXPECT_TRUE(TestError("{ \"minLength\" : null }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : false }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : \"length\" }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : {} }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : [] }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : 1.2 }", SEC_MIN_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : 12e-2 }", SEC_MIN_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"minLength\" : -1 }", SEC_MIN_LENGTH_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidPattern)
{
	EXPECT_TRUE(TestError("{ \"pattern\" : null }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(TestError("{ \"pattern\" : false }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(TestError("{ \"pattern\" : 0 }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(TestError("{ \"pattern\" : {} }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(TestError("{ \"pattern\" : [] }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(TestError("{ \"pattern\" : \"*\" }", SEC_PATTERN_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"pattern\" : \"[\" }", SEC_PATTERN_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidItems)
{
	EXPECT_TRUE(TestError("{ \"items\" : null }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : 0 }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : false }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : \"item\" }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [null] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [0] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [false] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [\"item\"] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, null] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, 0] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, false] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, \"item\"] }", SEC_ITEMS_ARRAY_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAdditionalItems)
{
	EXPECT_TRUE(TestError("{ \"additionalItems\" : null }", SEC_ADDITIONAL_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"additionalItems\" : 0 }", SEC_ADDITIONAL_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"additionalItems\" : \"item\" }", SEC_ADDITIONAL_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"additionalItems\" : [] }", SEC_ADDITIONAL_ITEMS_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaxItems)
{
	EXPECT_TRUE(TestError("{ \"maxItems\" : null }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : false }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : \"max\" }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : {} }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : [] }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : 1.2 }", SEC_MAX_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : 12e-2 }", SEC_MAX_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxItems\" : -1 }", SEC_MAX_ITEMS_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinItems)
{
	EXPECT_TRUE(TestError("{ \"minItems\" : null }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : false }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : \"min\" }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : {} }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : [] }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : 1.2 }", SEC_MIN_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : 12e-2 }", SEC_MIN_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"minItems\" : -1 }", SEC_MIN_ITEMS_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidUniqueItems)
{
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : null }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : 0 }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : \"unique\" }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : {} }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : [] }", SEC_UNIQUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaxProperties)
{
	EXPECT_TRUE(TestError("{ \"maxProperties\" : null }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : false }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : \"max\" }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : {} }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : [] }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : 1.2 }", SEC_MAX_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : 12e-2 }", SEC_MAX_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : -1 }", SEC_MAX_PROPERTIES_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinProperties)
{
	EXPECT_TRUE(TestError("{ \"minProperties\" : null }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : false }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : \"min\" }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : {} }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : [] }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : 1.2 }", SEC_MIN_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : 12e-2 }", SEC_MIN_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(TestError("{ \"minProperties\" : -1 }", SEC_MIN_PROPERTIES_VALUE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidRequired)
{
	EXPECT_TRUE(TestError("{ \"required\" : null }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : false }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : 0 }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : \"req\" }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : {} }", SEC_REQUIRED_FORMAT));
	// FIXME: "required" property should have at least one element
	//EXPECT_TRUE(TestError("{ \"required\" : [] }", SEC_SYNTAX));
	EXPECT_TRUE(TestError("{ \"required\" : [null] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [false] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [0] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [{}] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [[]] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", null] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", false] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", 0] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", {}] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", []] }", SEC_REQUIRED_ARRAY_FORMAT));
	// FIXME: "required" values must be unique
	//EXPECT_TRUE(TestError("{ \"required\" : [\"a\", \"a\"] }", SEC_SYNTAX));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAdditionalProperties)
{
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : null }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : 0 }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : \"item\" }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : [] }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidProperties)
{
	EXPECT_TRUE(TestError("{ \"properties\" : null }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : false }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : 0 }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : \"item\" }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : [] }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : null } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : false } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : 0 } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : \"hello\" } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : [] } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : null } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : false } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : 0 } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : \"hello\" } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : [] } }", SEC_PROPERTIES_OBJECT_FORMAT));
}

// TODO: Implement "patternProperties" propetry
//
//TEST_F(TestSchemaParsingErrorReporting, InvalidPatternProperties)
//{
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : null }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : false }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : 0 }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : \"item\" }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : [] }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : null } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : false } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : 0 } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : \"hello\" } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : [] } }", SEC_SYNTAX));
//	// TODO: Add test to check regex syntax
//}

// TODO: Implement "dependencies" property
//
//TEST_F(TestSchemaParsingErrorReporting, InvalidDependencies)
//{
//	EXPECT_TRUE(TestError("{ \"dependencies\" : null }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : false }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : 0 }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : \"dep\" }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : [] }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : null } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : false } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : 0 } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : \"dep\" } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [null] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [false] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [0] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [{}] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [[]] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", null] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", false] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", 0] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", {}] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", []] } }", SEC_SYNTAX));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", \"dep\"] } }", SEC_SYNTAX));
//}

TEST_F(TestSchemaParsingErrorReporting, InvalidEnum)
{
	EXPECT_TRUE(TestError("{ \"enum\" : null }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"enum\" : false }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"enum\" : 0 }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"enum\" : \"e\" }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"enum\" : {} }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(TestError("{ \"enum\" : [] }", SEC_ENUM_ARRAY_EMPTY));
	EXPECT_TRUE(TestError("{ \"enum\" : [null, null] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"enum\" : [\"hello\", null, \"hello\"] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"enum\" : [[], []] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"enum\" : [[null, 12], 12, [null, 12]] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"enum\" : [{}, {}] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(TestError("{ \"enum\" : [ {\"a\":null, \"b\":\"str\"}, null, {\"b\":\"str\", \"a\":null } ] }", SEC_ENUM_ARRAY_DUPLICATES));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAllOf)
{
	EXPECT_TRUE(TestError("{ \"allOf\" : null }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : false }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : 0 }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : \"a\" }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : {} }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [] }", SEC_ALL_OF_ARRAY_EMPTY));
	EXPECT_TRUE(TestError("{ \"allOf\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAnyOf)
{
	EXPECT_TRUE(TestError("{ \"anyOf\" : null }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : false }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : 0 }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : \"a\" }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : {} }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [] }", SEC_ANY_OF_ARRAY_EMPTY));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidOneOf)
{
	EXPECT_TRUE(TestError("{ \"oneOf\" : null }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : false }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : 0 }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : \"a\" }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : {} }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [] }", SEC_ONE_OF_ARRAY_EMPTY));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidNot)
{
	EXPECT_TRUE(TestError("{ \"not\" : null }", SEC_NOT_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : false }", SEC_NOT_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : 0 }", SEC_NOT_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : \"a\" }", SEC_NOT_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [] }", SEC_NOT_ARRAY_EMPTY));
	EXPECT_TRUE(TestError("{ \"not\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(TestError("{ \"not\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidDefinitions)
{
	EXPECT_TRUE(TestError("{ \"definitions\" : null }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : false }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : 0 }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : \"def\" }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : [] }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : null } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : false } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : 0 } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : \"hello\" } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : [] } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : null } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : false } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : 0 } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : \"hello\" } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : [] } }", SEC_DEFINITIONS_OBJECT_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidTitle)
{
	EXPECT_TRUE(TestError("{ \"title\" : null }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(TestError("{ \"title\" : false }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(TestError("{ \"title\" : 0 }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(TestError("{ \"title\" : {} }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(TestError("{ \"title\" : [] }", SEC_TITLE_FORMAT));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidDescription)
{
	EXPECT_TRUE(TestError("{ \"description\" : null }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(TestError("{ \"description\" : false }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(TestError("{ \"description\" : 0 }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(TestError("{ \"description\" : {} }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(TestError("{ \"description\" : [] }", SEC_DESCRIPTION_FORMAT));
}
