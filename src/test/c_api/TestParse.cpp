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
#include <fstream>
#include <cxx/JSchemaFile.h>

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

struct test_sax_context {
	int null_counter;
	int boolean_counter;
	int number_counter;
	int string_counter;
	int object_start_counter;
	int object_key_counter;
	int object_end_counter;
	int array_start_counter;
	int array_end_counter;
	PJSAXCallbacks callbacks;

	test_sax_context()
		: null_counter(0)
		, boolean_counter(0)
		, number_counter(0)
		, string_counter(0)
		, object_start_counter(0)
		, object_key_counter(0)
		, object_end_counter(0)
		, array_start_counter(0)
		, array_end_counter(0)
	{
		callbacks.m_objStart = jsax_object_start;
		callbacks.m_objKey = jsax_object_key;
		callbacks.m_objEnd = jsax_object_end;
		callbacks.m_arrStart = jsax_array_start;
		callbacks.m_arrEnd = jsax_array_end;
		callbacks.m_string = jsax_string;
		callbacks.m_number = jsax_number;
		callbacks.m_boolean = jsax_boolean;
		callbacks.m_null = jsax_null;
	}

	static int jsax_null(JSAXContextRef ctxt) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->null_counter++;
		return 1;
	}

	static int jsax_boolean(JSAXContextRef ctxt, bool value) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->boolean_counter++;
		return 1;
	}

	static int jsax_number(JSAXContextRef ctxt, const char *number, size_t numberLen) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->number_counter++;
		return 1;
	}

	static int jsax_string(JSAXContextRef ctxt, const char *string, size_t stringLen) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->string_counter++;
		return 1;
	}

	static int jsax_object_start(JSAXContextRef ctxt) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->object_start_counter++;
		return 1;
	}

	static int jsax_object_key(JSAXContextRef ctxt, const char *key, size_t keyLen) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->object_key_counter++;
		return 1;
	}

	static int jsax_object_end(JSAXContextRef ctxt) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->object_end_counter++;
		return 1;
	}

	static int jsax_array_start(JSAXContextRef ctxt) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->array_start_counter++;
		return 1;
	}

	static int jsax_array_end(JSAXContextRef ctxt) {
		reinterpret_cast<test_sax_context*>(jsax_getContext(ctxt))->array_end_counter++;
		return 1;
	}

};

void ReadFileToString(const std::string& fileName, std::string& dst)
{
	std::ifstream file(fileName);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + fileName);

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	dst.resize(size);
	file.seekg(0);
	file.read(&dst[0], size);
}

TEST(TestParse, saxparser)
{
	test_sax_context context;

	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	jschema_ref schema = jschema_parse_file("../schemas/parse/test_stream_parser.schema", NULL);
	ASSERT_FALSE(NULL == schema);

	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	jsaxparser_ref parser = jsaxparser_create(&schemaInfo, &context.callbacks, &context);

	ASSERT_FALSE(parser == NULL);

	const char* start = json_str.c_str();
	const char* end = json_str.c_str() + json_str.length();
	for (const char* i = start ; i !=  end ; ++i) {
		if (!jsaxparser_feed(parser, i, 1)) {
			const char* error = jsaxparser_get_error(parser);
			while(!error) {break;}
			break;
		}
	}

	if (!jsaxparser_end(parser)) {
		const char* error = jsaxparser_get_error(parser);
		while(!error) {break;}
	}

	jsaxparser_release(&parser);

	jschema_release(&schema);

	EXPECT_EQ(1, context.null_counter);
	EXPECT_EQ(1, context.boolean_counter);
	EXPECT_EQ(2, context.string_counter);
	EXPECT_EQ(2, context.number_counter);
	EXPECT_EQ(1, context.array_start_counter);
	EXPECT_EQ(1, context.array_end_counter);
	EXPECT_EQ(1, context.object_start_counter);
	EXPECT_EQ(5, context.object_key_counter);
	EXPECT_EQ(1, context.object_end_counter);
}

raw_buffer from_str_to_buffer(const char* str)
{
	raw_buffer ret;
	ret.m_str = str;
	ret.m_len = strlen(str);

	return ret;
}

TEST(TestParse, domparser)
{

	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	jschema_ref schema = jschema_parse_file("../schemas/parse/test_stream_parser.schema", NULL);
	ASSERT_FALSE(NULL == schema);

	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	jdomparser_ref parser = jdomparser_create(&schemaInfo, 0);
	ASSERT_FALSE(parser == NULL);

	const char* start = json_str.c_str();
	const char* end = json_str.c_str() + json_str.length();
	for (const char* i = start ; i !=  end ; ++i) {
		if (!jdomparser_feed(parser, i, 1)) {
			const char* error = jdomparser_get_error(parser);
			while(!error) {break;}
			break;
		}
	}

	if (!jdomparser_end(parser)) {
		const char* error = jdomparser_get_error(parser);
		while(!error) {break;}
	}

	jvalue_ref jval = jdomparser_get_result(parser);

	jdomparser_release(&parser);

	ASSERT_TRUE(jobject_get_exists(jval, from_str_to_buffer("null"), NULL));
	ASSERT_TRUE(jobject_get_exists(jval, from_str_to_buffer("bool"), NULL));
	ASSERT_TRUE(jobject_get_exists(jval, from_str_to_buffer("number"), NULL));
	ASSERT_TRUE(jobject_get_exists(jval, from_str_to_buffer("string"), NULL));
	ASSERT_TRUE(jobject_get_exists(jval, from_str_to_buffer("array"), NULL));

	bool boolValue = false;
	jboolean_get(jobject_get(jval, from_str_to_buffer("bool")), &boolValue);
	EXPECT_EQ(true, boolValue);

	double numValue = 0;
	jnumber_get_f64(jobject_get(jval, from_str_to_buffer("number")), &numValue);
	EXPECT_EQ(1.1, numValue);

	raw_buffer str = jstring_get_fast(jobject_get(jval, from_str_to_buffer("string")));
	EXPECT_EQ(std::string("asd"), std::string(str.m_str, str.m_len));

	jvalue_ref array = jobject_get(jval, from_str_to_buffer("array"));
	ASSERT_EQ(2, jarray_size(array));

	int array_elem_int = 0;
	jnumber_get_i32(jarray_get(array, 0), &array_elem_int);
	EXPECT_EQ(2, array_elem_int);

	str = jstring_get_fast(jarray_get(array, 1));
	EXPECT_EQ(std::string("qwerty"), std::string(str.m_str, str.m_len));

	j_release(&jval);
	jschema_release(&schema);
}
