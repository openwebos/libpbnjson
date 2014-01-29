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
#include <iostream>
#include <cassert>
#include <limits>
#include <execinfo.h>
#include <pbnjson.h>
#include <memory>
#include <algorithm>

void j_release_ref(jvalue * val) {
	j_release(&val);
}

void jschema_release_ref(jschema_ref val) {
	jschema_release(&val);
}

template<class Val>
struct jptr_generic : public std::shared_ptr<Val>
{
	template<class Del>
	jptr_generic(Val * val, Del del)
		: std::shared_ptr<Val>(val, del)
	{
	}

	const jptr_generic & operator=(Val * jval) {
		reset(jval);
		return *this;
	}
};

struct jptr_value : public jptr_generic<jvalue>
{
	jptr_value()
		: jptr_generic(0, j_release_ref)
	{

	}

	jptr_value(jvalue * val)
		: jptr_generic(val, j_release_ref)
	{
	}

	operator jvalue_ref() {
		return get();
	}
};

struct jptr_schema : public jptr_generic<jschema>
{
	jptr_schema()
		: jptr_generic(0, jschema_release_ref)
	{

	}

	jptr_schema(jschema * val)
		: jptr_generic(val, jschema_release_ref)
	{
	}

	operator jschema_ref() {
		return get();
	}
};

TEST(TestParse, testInvalidJson) {
	JSchemaInfo schemaInfo;

	jschema_info_init(&schemaInfo, jschema_all(), NULL, NULL);

	jptr_value parsed{ jdom_parse(j_cstr_to_buffer("}} ..bad value.. {{"), DOMOPT_NOOPT, &schemaInfo) };
	ASSERT_TRUE( jis_null(parsed) );
	ASSERT_FALSE( jis_valid(parsed) );
}

TEST(TestParse, testParseDoubleAccuracy) {

	std::string jsonRaw("{\"errorCode\":0,\"timestamp\":1.268340607585E12,\"latitude\":37.390067,\"longitude\":-122.037626,\"horizAccuracy\":150,\"heading\":0,\"velocity\":0,\"altitude\":0,\"vertAccuracy\":0}");
	JSchemaInfo schemaInfo;

	double longitude;

	jschema_info_init(&schemaInfo, jschema_all(), NULL, NULL);

	jptr_value parsed{ jdom_parse(j_cstr_to_buffer(jsonRaw.c_str()), DOMOPT_NOOPT, &schemaInfo) };
	ASSERT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jis_number(jobject_get(parsed, J_CSTR_TO_BUF("longitude"))));
	EXPECT_EQ((ConversionResultFlags)CONV_OK, jnumber_get_f64(jobject_get(parsed, J_CSTR_TO_BUF("longitude")), &longitude));
	EXPECT_EQ(-122.037626, longitude);
}

static bool identical(jvalue_ref obj1, jvalue_ref obj2)
{
	if (jis_object(obj1))
	{
		if (!jis_object(obj2))
			return false;


		int numKeys1 = jobject_size(obj1);
		int numKeys2 = jobject_size(obj2);

		if (numKeys1 != numKeys2)
			return false;

		jobject_iter iter;
		jobject_iter_init(&iter, obj1);
		jobject_key_value keyval;

		while (jobject_iter_next(&iter, &keyval))
		{
			jvalue_ref obj2Val;
			if (!jobject_get_exists(obj2, jstring_get_fast(keyval.key), &obj2Val))
				return false;

			if (!identical(keyval.value, obj2Val))
				return false;
		}

		return true;
	}

	if (jis_array(obj1)) {
		if (!jis_array(obj2))
			return false;

		if (jarray_size(obj1) != jarray_size(obj2))
			return false;

		int ni = jarray_size(obj1);
		for (int i = 0; i < ni; i++) {
			if (!identical(jarray_get(obj1, i), jarray_get(obj2, i)))
				return false;
		}

		return true;
	}

	if (jis_string(obj1)) {
		if (!jis_string(obj2))
			return false;

		return jstring_equal(obj1, obj2);
	}

	if (jis_number(obj1)) {
		if (!jis_number(obj2))
			return false;

		return jnumber_compare(obj1, obj2) == 0;
	}

	if (jis_boolean(obj1)) {
		if (!jis_boolean(obj2))
			return false;

		bool b1, b2;
		return jboolean_get(obj1, &b1) == CONV_OK &&
		       jboolean_get(obj2, &b2) == CONV_OK &&
		       b1 == b2;
	}

	if (jis_null(obj1)) {
		if (!jis_null(obj2))
			return false;

		return true;
	}

	abort();
	return false;
}

void TestParse_testParseFile(const std::string &fileNameSignature)
{
	std::string jsonInput = fileNameSignature + ".json";
	std::string jsonSchema = fileNameSignature + ".schema";

	jptr_schema schema = jschema_parse_file(jsonSchema.c_str(), NULL);
	ASSERT_TRUE (schema != NULL);

	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	jptr_value inputNoMMap { jdom_parse_file(jsonInput.c_str(), &schemaInfo, JFileOptNoOpt) };
	EXPECT_FALSE(jis_null(inputNoMMap));

	jptr_value inputMMap { jdom_parse_file(jsonInput.c_str(), &schemaInfo, JFileOptMMap) };
	EXPECT_FALSE(jis_null(inputMMap));

	EXPECT_TRUE(identical(inputNoMMap, inputMMap));
}

TEST(TestParse, testParseFile)
{
	std::vector<std::string> tasks = {"file_parse_test"};
	for (const auto &task : tasks) TestParse_testParseFile(task);
}
