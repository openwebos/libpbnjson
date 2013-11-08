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
	bool errorOccured;
	string errorMsg;

	TestSchemaParsingErrorReporting()
	{
		error.m_parser = &OnError;
		error.m_ctxt = this;
	}

	virtual void SetUp()
	{
		errorOccured = false;
		errorMsg = "";
	}

	static bool OnError(void *ctxt, JSAXContextRef parseCtxt)
	{
		TestSchemaParsingErrorReporting *t = reinterpret_cast<TestSchemaParsingErrorReporting *>(ctxt);
		assert(t);
		t->errorOccured = true;
		t->errorMsg = parseCtxt->errorDescription;
		return false;
	}

	bool TestError(const char *schemaStr, const char *errorMsg)
	{
		SetUp();
		auto schema = mk_ptr(jschema_parse(j_cstr_to_buffer(schemaStr), JSCHEMA_DOM_NOOPT, &error));
		if (errorMsg)
		{
			EXPECT_EQ(this->errorMsg, errorMsg);
		}
		return errorOccured;
	}
};

TEST_F(TestSchemaParsingErrorReporting, InvalidJson)
{
	EXPECT_TRUE(TestError("]", NULL));
	EXPECT_TRUE(TestError("{]", NULL));
	EXPECT_TRUE(TestError("[qwe]", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidSchemaType1)
{
	// TODO: Generalize schema parsing error messages and update unit test to check them
	//       Also more detailed error reporting would be nice (like "Invalid "type" parameter type. Should be string")
	EXPECT_TRUE(TestError("{ \"type\" : null }", /*"JSON schema parsing failed"*/ NULL));
	EXPECT_TRUE(TestError("{ \"type\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : \"invalid_type\" }", /*"Invalid type"*/ NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [null] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [0] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [false] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [{}] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [[]] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"invalid_type\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", null] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", 0 ] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", false ] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", {} ] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", [] ] }", NULL));
	EXPECT_TRUE(TestError("{ \"type\" : [\"array\", \"invalid_type\"] }", NULL));
	// FIXME: elements of the "type" array should be unique
	//EXPECT_TRUE(TestError("{ \"type\" : [\"array\", \"array\"] }", NULL));
}

// TODO: Implement "multipleOf" property
//
//TEST_F(TestSchemaParsingErrorReporting, InvalidMultipleOf)
//{
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : null }", NULL));
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : false }", NULL));
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : \"mul\" }", NULL));
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : {} }", NULL));
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : [] }", NULL));
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : -1.2 }", NULL));
//	EXPECT_TRUE(TestError("{ \"multipleOf\" : 0 }", NULL));
//}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaximum)
{
	EXPECT_TRUE(TestError("{ \"maximum\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"maximum\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"maximum\" : \"max\" }", NULL));
	EXPECT_TRUE(TestError("{ \"maximum\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"maximum\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : \"exclusive\" }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : [] }", NULL));
	// FIXME: if "exclusiveMaximum" is present "maximum" should be present also
	//EXPECT_TRUE(TestError("{ \"exclusiveMaximum\" : false }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinimum)
{
	EXPECT_TRUE(TestError("{ \"minimum\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"minimum\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"minimum\" : \"min\" }", NULL));
	EXPECT_TRUE(TestError("{ \"minimum\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"minimum\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : \"exclusive\" }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : [] }", NULL));
	// FIXME: if "exclusiveMinimum" is present "minimum" should be present also
	//EXPECT_TRUE(TestError("{ \"exclusiveMinimum\" : false }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaxLength)
{
	EXPECT_TRUE(TestError("{ \"maxLength\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : \"length\" }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : 1.2 }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : 12e-2 }", NULL));
	EXPECT_TRUE(TestError("{ \"maxLength\" : -1 }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinLength)
{
	EXPECT_TRUE(TestError("{ \"minLength\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : \"length\" }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : 1.2 }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : 12e-2 }", NULL));
	EXPECT_TRUE(TestError("{ \"minLength\" : -1 }", NULL));
}

// TODO: Implement "pattern" property
//TEST_F(TestSchemaParsingErrorReporting, InvalidPattern)
//{
//	EXPECT_TRUE(TestError("{ \"pattern\" : null }", NULL));
//	EXPECT_TRUE(TestError("{ \"pattern\" : false }", NULL));
//	EXPECT_TRUE(TestError("{ \"pattern\" : 0 }", NULL));
//	EXPECT_TRUE(TestError("{ \"pattern\" : {} }", NULL));
//	EXPECT_TRUE(TestError("{ \"pattern\" : [] }", NULL));
//  // TODO: Add test to check regex syntax
//}

TEST_F(TestSchemaParsingErrorReporting, InvalidItems)
{
	EXPECT_TRUE(TestError("{ \"items\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : \"item\" }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [null] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [0] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [false] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [\"item\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, null] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, 0] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, false] }", NULL));
	EXPECT_TRUE(TestError("{ \"items\" : [{}, \"item\"] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAdditionalItems)
{
	EXPECT_TRUE(TestError("{ \"additionalItems\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"additionalItems\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"additionalItems\" : \"item\" }", NULL));
	EXPECT_TRUE(TestError("{ \"additionalItems\" : [] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaxItems)
{
	EXPECT_TRUE(TestError("{ \"maxItems\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : \"max\" }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : 1.2 }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : 12e-2 }", NULL));
	EXPECT_TRUE(TestError("{ \"maxItems\" : -1 }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinItems)
{
	EXPECT_TRUE(TestError("{ \"minItems\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : \"min\" }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : 1.2 }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : 12e-2 }", NULL));
	EXPECT_TRUE(TestError("{ \"minItems\" : -1 }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidUniqueItems)
{
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : \"unique\" }", NULL));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"uniqueItems\" : [] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMaxProperties)
{
	EXPECT_TRUE(TestError("{ \"maxProperties\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : \"max\" }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : 1.2 }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : 12e-2 }", NULL));
	EXPECT_TRUE(TestError("{ \"maxProperties\" : -1 }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidMinProperties)
{
	EXPECT_TRUE(TestError("{ \"minProperties\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : \"min\" }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : 1.2 }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : 12e-2 }", NULL));
	EXPECT_TRUE(TestError("{ \"minProperties\" : -1 }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidRequired)
{
	EXPECT_TRUE(TestError("{ \"required\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : \"req\" }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : {} }", NULL));
	// FIXME: "required" property should have at least one element
	//EXPECT_TRUE(TestError("{ \"required\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [null] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [false] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [0] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [{}] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [[]] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", null] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", false] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", 0] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", {}] }", NULL));
	EXPECT_TRUE(TestError("{ \"required\" : [\"a\", []] }", NULL));
	// FIXME: "required" values must be unique
	//EXPECT_TRUE(TestError("{ \"required\" : [\"a\", \"a\"] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAdditionalProperties)
{
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : \"item\" }", NULL));
	EXPECT_TRUE(TestError("{ \"additionalProperties\" : [] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidProperties)
{
	EXPECT_TRUE(TestError("{ \"properties\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : \"item\" }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : null } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : false } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : 0 } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : \"hello\" } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : [] } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : null } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : false } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : 0 } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : \"hello\" } }", NULL));
	EXPECT_TRUE(TestError("{ \"properties\" : { \"a\" : {}, \"b\" : [] } }", NULL));
}

// TODO: Implement "patternProperties" propetry
//
//TEST_F(TestSchemaParsingErrorReporting, InvalidPatternProperties)
//{
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : null }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : false }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : 0 }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : \"item\" }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : [] }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : null } }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : false } }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : 0 } }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : \"hello\" } }", NULL));
//	EXPECT_TRUE(TestError("{ \"patternProperties\" : { \"a\" : [] } }", NULL));
//	// TODO: Add test to check regex syntax
//}

// TODO: Implement "dependencies" property
//
//TEST_F(TestSchemaParsingErrorReporting, InvalidDependencies)
//{
//	EXPECT_TRUE(TestError("{ \"dependencies\" : null }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : false }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : 0 }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : \"dep\" }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : [] }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : null } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : false } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : 0 } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : \"dep\" } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [null] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [false] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [0] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [{}] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [[]] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", null] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", false] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", 0] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", {}] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", []] } }", NULL));
//	EXPECT_TRUE(TestError("{ \"dependencies\" : { \"a\" : [\"dep\", \"dep\"] } }", NULL));
//}

TEST_F(TestSchemaParsingErrorReporting, InvalidEnum)
{
	EXPECT_TRUE(TestError("{ \"enum\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"enum\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"enum\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"enum\" : \"e\" }", NULL));
	EXPECT_TRUE(TestError("{ \"enum\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"enum\" : [] }", NULL));
	// FIXME: "enum" values should be unique
	//EXPECT_TRUE(TestError("{ \"enum\" : [null, null] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAllOf)
{
	EXPECT_TRUE(TestError("{ \"allOf\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : \"a\" }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [null] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [false] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [0] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [\"a\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [[]] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, null] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, false] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, 0] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, \"a\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"allOf\" : [{}, []] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidAnyOf)
{
	EXPECT_TRUE(TestError("{ \"anyOf\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : \"a\" }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [null] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [false] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [0] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [\"a\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [[]] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, null] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, false] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, 0] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, \"a\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"anyOf\" : [{}, []] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidOneOf)
{
	EXPECT_TRUE(TestError("{ \"oneOf\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : \"a\" }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [null] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [false] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [0] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [\"a\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [[]] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, null] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, false] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, 0] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, \"a\"] }", NULL));
	EXPECT_TRUE(TestError("{ \"oneOf\" : [{}, []] }", NULL));
}

// TODO: Implement "not" property
//
//TEST_F(TestSchemaParsingErrorReporting, InvalidNot)
//{
//	EXPECT_TRUE(TestError("{ \"not\" : null }", NULL));
//	EXPECT_TRUE(TestError("{ \"not\" : false }", NULL));
//	EXPECT_TRUE(TestError("{ \"not\" : 0 }", NULL));
//	EXPECT_TRUE(TestError("{ \"not\" : \"a\" }", NULL));
//	EXPECT_TRUE(TestError("{ \"not\" : [] }", NULL));
//}

TEST_F(TestSchemaParsingErrorReporting, InvalidDefinitions)
{
	EXPECT_TRUE(TestError("{ \"definitions\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : \"def\" }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : [] }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : null } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : false } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : 0 } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : \"hello\" } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : [] } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : null } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : false } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : 0 } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : \"hello\" } }", NULL));
	EXPECT_TRUE(TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : [] } }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidTitle)
{
	EXPECT_TRUE(TestError("{ \"title\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"title\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"title\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"title\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"title\" : [] }", NULL));
}

TEST_F(TestSchemaParsingErrorReporting, InvalidDescription)
{
	EXPECT_TRUE(TestError("{ \"description\" : null }", NULL));
	EXPECT_TRUE(TestError("{ \"description\" : false }", NULL));
	EXPECT_TRUE(TestError("{ \"description\" : 0 }", NULL));
	EXPECT_TRUE(TestError("{ \"description\" : {} }", NULL));
	EXPECT_TRUE(TestError("{ \"description\" : [] }", NULL));
}
