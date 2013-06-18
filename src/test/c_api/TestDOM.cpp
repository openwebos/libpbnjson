// @@@LICENSE
//
//      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
//      Copyright (c) 2013 LG Electronics, Inc.
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

using namespace std;

class Manager
{
private:
	vector<jvalue_ref> managed;

public:
	~Manager()
	{
		for (auto &val: managed)
			j_release(&val);
	}

	jvalue_ref operator()(jvalue_ref val)
	{
		managed.push_back(val);
		return val;
	}

} g_manager;

static jvalue_ref manage(jvalue_ref val)
{
	return g_manager(val);
}

TEST(TestDOM, ObjectSimple)
{
	ASSERT_FALSE(jis_null(manage(J_CSTR_TO_JVAL("abc"))));
	ASSERT_FALSE(jis_null(manage(J_CSTR_TO_JVAL("def"))));
	ASSERT_FALSE(jis_null(manage(jnumber_create(J_CSTR_TO_BUF("5463")))));

	jvalue_ref simpleObject = manage(jobject_create_var(
		jkeyval(J_CSTR_TO_JVAL("abc"), J_CSTR_TO_JVAL("def")),
		jkeyval(J_CSTR_TO_JVAL("def"), jnumber_create(J_CSTR_TO_BUF("5463"))),
		J_END_OBJ_DECL
	));

	ASSERT_TRUE(jis_object(simpleObject));
	jvalue_ref jstr(0);
	EXPECT_TRUE(jobject_get_exists(simpleObject, J_CSTR_TO_BUF("abc"), &jstr));
	EXPECT_STREQ(jstring_get_fast(jstr).m_str, "def");

	jvalue_ref jnum(0);
	EXPECT_TRUE(jobject_get_exists(simpleObject, J_CSTR_TO_BUF("def"), &jnum));
	int32_t num(-1);
	EXPECT_EQ(jnumber_get_i32(jnum, &num), CONV_OK);
    EXPECT_EQ(num, 5463);
}

// sanity check that assumptions about limits of double storage
// are correct
static const int64_t maxDblPrecision = 0x1FFFFFFFFFFFFFLL;
static const int64_t minDblPrecision = -0x1FFFFFFFFFFFFFLL;
static const int64_t positiveOutsideDblPrecision = maxDblPrecision + 2; // +1 doesn't work because it's the same (it truncates a 0)
static const int64_t negativeOutsideDblPrecision = minDblPrecision - 2; // +1 doesn't work because it's the same (it truncates a 0)
static const int64_t maxInt32 = std::numeric_limits<int32_t>::max();
static const int64_t minInt32 = std::numeric_limits<int32_t>::min();
static const char weirdString[] = "long and complicated \" string ' with \" $*U@*(&#(@*&";
static const char veryLargeNumber[] = "645458489754321564894654151561684894456464513215648946543132189489461321684.2345646544e509";

TEST(TestDOM, SanityCheck)
{
	double check = (double)maxDblPrecision;
	EXPECT_EQ((int64_t)check, maxDblPrecision);

	check = (double)minDblPrecision;
	EXPECT_EQ((int64_t)check, minDblPrecision);

	check = (double)(positiveOutsideDblPrecision);
	EXPECT_NE((int64_t)check, positiveOutsideDblPrecision);

	check = (double)(negativeOutsideDblPrecision);
	EXPECT_NE((int64_t)check, negativeOutsideDblPrecision);

	EXPECT_TRUE(std::numeric_limits<double>::has_quiet_NaN);
	EXPECT_TRUE(std::numeric_limits<double>::has_signaling_NaN);
}

TEST(TestDOM, ObjectComplex)
{
	// unfortunately, C++ doesn't allow us to use J_CSTR_TO_JVAL which is what I would use
	// for string literals under C.
	jvalue_ref complexObject = manage (jobject_create_var(
		// J_CSTR_TO_JVAL or J_CSTR_TO_JVAL is interchangeable only if you use string literals.
		// J_CSTR_TO_JVAL is going to be faster as your string gets larger
		jkeyval(J_CSTR_TO_JVAL("bool1"), jboolean_create(true)),
		jkeyval(J_CSTR_TO_JVAL("bool2"), jboolean_create(false)),
		jkeyval(J_CSTR_TO_JVAL("numi32_1"), jnumber_create_i32(0)),
		jkeyval(J_CSTR_TO_JVAL("numi32_2"), jnumber_create_i32(-50)),
		jkeyval(J_CSTR_TO_JVAL("numi32_3"), jnumber_create_i32(12345323)),
		jkeyval(J_CSTR_TO_JVAL("numi64_1"), jnumber_create_i64(maxInt32 + 1)),
		jkeyval(J_CSTR_TO_JVAL("numi64_2"), jnumber_create_i64(minInt32 - 1)),
		jkeyval(J_CSTR_TO_JVAL("numi64_3"), jnumber_create_i64(0)),
		jkeyval(J_CSTR_TO_JVAL("numi64_4"), jnumber_create_i64(maxDblPrecision)),
		jkeyval(J_CSTR_TO_JVAL("numi64_5"), jnumber_create_i64(minDblPrecision)),
		jkeyval(J_CSTR_TO_JVAL("numi64_6"), jnumber_create_i64(positiveOutsideDblPrecision)),
		jkeyval(J_CSTR_TO_JVAL("numi64_7"), jnumber_create_i64(negativeOutsideDblPrecision)),
		jkeyval(J_CSTR_TO_JVAL("numf64_1"), jnumber_create_f64(0.45642156489)),
		jkeyval(J_CSTR_TO_JVAL("numf64_2"), jnumber_create_f64(-54897864.14)),
		jkeyval(J_CSTR_TO_JVAL("numf64_3"), jnumber_create_f64(-54897864)),
		jkeyval(J_CSTR_TO_JVAL("numf64_4"), jnumber_create_f64(std::numeric_limits<double>::infinity())),
		jkeyval(J_CSTR_TO_JVAL("numf64_5"), jnumber_create_f64(-std::numeric_limits<double>::infinity())),
		jkeyval(J_CSTR_TO_JVAL("numf64_6"), jnumber_create_f64(-std::numeric_limits<double>::quiet_NaN())),
		jkeyval(J_CSTR_TO_JVAL("str1"), jnull()),
		jkeyval(J_CSTR_TO_JVAL("str2"), jnull()),
		jkeyval(J_CSTR_TO_JVAL("str3"), jstring_empty()),
		jkeyval(J_CSTR_TO_JVAL("str4"), J_CSTR_TO_JVAL("foo")),
		jkeyval(J_CSTR_TO_JVAL("str5"), J_CSTR_TO_JVAL(weirdString)),
		jkeyval(J_CSTR_TO_JVAL("obj1"),
				jobject_create_var(
						jkeyval(J_CSTR_TO_JVAL("num_1"), jnumber_create(J_CSTR_TO_BUF("64.234"))),
						jkeyval(J_CSTR_TO_JVAL("num_2"), jnumber_create(J_CSTR_TO_BUF(veryLargeNumber))),
						J_END_OBJ_DECL
				)
		),
		J_END_OBJ_DECL
	));

	jvalue_ref jbool = jobject_get(complexObject, J_CSTR_TO_BUF("bool1"));
	EXPECT_TRUE(jis_boolean(jbool));
	bool bool_val(false);
	EXPECT_EQ(jboolean_get(jbool, &bool_val), CONV_OK);
	EXPECT_EQ(bool_val, true);

	jbool = jobject_get(complexObject, J_CSTR_TO_BUF("bool2"));
	EXPECT_TRUE(jis_boolean(jbool));
	EXPECT_EQ(jboolean_get(jbool, &bool_val), CONV_OK);
	EXPECT_EQ(bool_val, false);

	jvalue_ref jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi32_1"));
	EXPECT_TRUE(jis_number(jnum));
	int32_t i32(-1);
	EXPECT_EQ(jnumber_get_i32(jnum, &i32), CONV_OK);
	EXPECT_EQ(i32, 0);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi32_2"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i32(jnum, &i32), CONV_OK);
	EXPECT_EQ(i32, -50);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi32_3"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i32(jnum, &i32), CONV_OK);
	EXPECT_EQ(i32, 12345323);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_1"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i32(jnum, &i32), CONV_POSITIVE_OVERFLOW);
	int64_t i64(-1);
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, maxInt32 + 1);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_2"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i32(jnum, &i32), CONV_NEGATIVE_OVERFLOW);
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, minInt32 - 1);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_3"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, 0);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_4"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, maxDblPrecision);
	double dbl;
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_OK);
	EXPECT_EQ(dbl, (double)maxDblPrecision);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_5"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, minDblPrecision);
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_OK);
	EXPECT_EQ(dbl, (double)minDblPrecision);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_6"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, positiveOutsideDblPrecision);
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_PRECISION_LOSS);
	EXPECT_EQ(dbl, (double)positiveOutsideDblPrecision);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numi64_7"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, negativeOutsideDblPrecision);
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_PRECISION_LOSS);
	EXPECT_EQ(dbl, (double)negativeOutsideDblPrecision);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numf64_1"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_OK);
	EXPECT_EQ(dbl, 0.45642156489);
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_PRECISION_LOSS);
	EXPECT_EQ(i64, 0);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numf64_2"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_OK);
	EXPECT_EQ(dbl, -54897864.14);
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_PRECISION_LOSS);
	EXPECT_EQ(i64, -54897864);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numf64_3"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_OK);
	EXPECT_EQ(dbl, -54897864);
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_OK);
	EXPECT_EQ(i64, -54897864);

	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numf64_4"));
	EXPECT_TRUE(jis_null(jnum));  // + inf
	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numf64_5"));
	EXPECT_TRUE(jis_null(jnum));  // - inf
	jnum = jobject_get(complexObject, J_CSTR_TO_BUF("numf64_6"));
	EXPECT_TRUE(jis_null(jnum));  // NaN


	jvalue_ref jstr = jobject_get(complexObject, J_CSTR_TO_BUF("str1"));
	EXPECT_TRUE(jis_null(jstr));
	jstr = jobject_get(complexObject, J_CSTR_TO_BUF("str2"));
	EXPECT_TRUE(jis_null(jstr));

	jstr = jobject_get(complexObject, J_CSTR_TO_BUF("str3"));
	EXPECT_TRUE(jis_string(jstr));

	auto string_from_jval = [](jvalue_ref jstr) -> string
	{
		raw_buffer buf = jstring_get_fast(jstr);
		return string(buf.m_str, buf.m_str + buf.m_len);
	};

	EXPECT_TRUE(string_from_jval(jstr).empty());

	jstr = jobject_get(complexObject, J_CSTR_TO_BUF("str4"));
	EXPECT_TRUE(jis_string(jstr));
	EXPECT_EQ(string_from_jval(jstr), string{"foo"});

	jstr = jobject_get(complexObject, J_CSTR_TO_BUF("str5"));
	EXPECT_TRUE(jis_string(jstr));
	EXPECT_EQ(string_from_jval(jstr), string{weirdString});

	jvalue_ref jobj = jobject_get(complexObject, J_CSTR_TO_BUF("obj1"));
	EXPECT_TRUE(jis_object(jobj));
	jnum = jobject_get(jobj, J_CSTR_TO_BUF("num_1"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_PRECISION_LOSS);
	EXPECT_EQ(i64, 64);
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_OK);
	EXPECT_EQ(dbl, 64.234);

	jnum = jobject_get(jobj, J_CSTR_TO_BUF("num_2"));
	EXPECT_TRUE(jis_number(jnum));
	EXPECT_EQ(jnumber_get_i64(jnum, &i64), CONV_POSITIVE_OVERFLOW | CONV_PRECISION_LOSS);
	EXPECT_EQ(i64, std::numeric_limits<int64_t>::max());
	EXPECT_EQ(jnumber_get_f64(jnum, &dbl), CONV_POSITIVE_OVERFLOW);
	EXPECT_EQ(dbl, std::numeric_limits<double>::infinity());
	raw_buffer raw;
	EXPECT_EQ(jnumber_get_raw(jnum, &raw), CONV_OK);
	EXPECT_EQ(string(raw.m_str, raw.m_str + raw.m_len), string(veryLargeNumber));
}

TEST(TestDOM, ObjectPut)
{
	jvalue_ref obj = manage(jobject_create());

	// name collision
	ASSERT_TRUE(jobject_put(obj, J_CSTR_TO_JVAL("test1"), jnumber_create_i32(5)));
	ASSERT_TRUE(jobject_put(obj, J_CSTR_TO_JVAL("test1"), J_CSTR_TO_JVAL("test2")));

	// replacement of object
	// valgrind will fail if this was done improperly
	jvalue_ref val = jobject_get(obj, J_CSTR_TO_BUF("test1"));
	EXPECT_TRUE(jis_string(val));

	// should be the same pointer since I used a 0-copy string
	// (assuming the c-compiler interns strings)
	EXPECT_STREQ(jstring_get_fast(val).m_str, "test2");
}

TEST(TestDOM, ArraySimple)
{
	jvalue_ref simple_arr = manage(jarray_create_var(NULL,
			J_CSTR_TO_JVAL("index 0"),
			jnumber_create(J_CSTR_TO_BUF("1")),
			jnumber_create_i32(2),
			jboolean_create(false),
			jboolean_create(true),
			jnull(),
			J_CSTR_TO_JVAL(""),
			jnumber_create_f64(7),
			J_END_ARRAY_DECL));

	ASSERT_EQ(jarray_size(simple_arr), 8);

	jvalue_ref val = jarray_get(simple_arr, 0);
	EXPECT_TRUE(jis_string(val));
	EXPECT_STREQ(jstring_get_fast(val).m_str, "index 0");

	val = jarray_get(simple_arr, 1);
	EXPECT_TRUE(jis_number(val));
	int32_t i32{-1};
	EXPECT_EQ(jnumber_get_i32(val, &i32), CONV_OK);
	EXPECT_EQ(i32, 1);

	val = jarray_get(simple_arr, 2);
	EXPECT_TRUE(jis_number(val));
	EXPECT_EQ(jnumber_get_i32(val, &i32), CONV_OK);
	EXPECT_EQ(i32, 2);

	val = jarray_get(simple_arr, 3);
	EXPECT_TRUE(jis_boolean(val));
	bool bool_val(true);
	EXPECT_EQ(jboolean_get(val, &bool_val), CONV_OK);
	EXPECT_EQ(bool_val, false);

	val = jarray_get(simple_arr, 4);
	EXPECT_TRUE(jis_boolean(val));
	EXPECT_EQ(jboolean_get(val, &bool_val), CONV_OK);
	EXPECT_EQ(bool_val, true);

	val = jarray_get(simple_arr, 5);
	EXPECT_TRUE(jis_null(val));

	val = jarray_get(simple_arr, 6);
	EXPECT_TRUE(jis_string(val));
	EXPECT_EQ(jstring_get_fast(val).m_len, 0);
	raw_buffer buf = jstring_get(val);
	EXPECT_STREQ(buf.m_str, "");
	EXPECT_EQ(buf.m_len, 0);
	free((void *)buf.m_str); // FIXME!!!

	val = jarray_get(simple_arr, 7);
	EXPECT_TRUE(jis_number(val));
	EXPECT_EQ(jnumber_get_i32(val, &i32), CONV_OK);
	EXPECT_EQ(i32, 7);
}

TEST(TestDOM, ArrayComplex)
{
	jvalue_ref arr = manage(jarray_create(NULL));

	ASSERT_TRUE(jis_array(arr));

	for (int32_t i = 0; i < 100; i++)
	{
		jarray_append(arr, jnumber_create_i32(i));
		EXPECT_EQ(jarray_size(arr), i + 1);

		jvalue_ref child = jarray_get(arr, i);
		EXPECT_TRUE(jis_number(child));
		int32_t i32(-1);
		EXPECT_EQ(jnumber_get_i32(child, &i32), CONV_OK);
		EXPECT_EQ(i32, i);

		if (i > 20)
			EXPECT_TRUE(jis_number(jarray_get(arr, 20)));
	}

	EXPECT_EQ(jarray_size(arr), 100);

	for (int32_t i = 0; i < 100; i++)
	{
		jvalue_ref child = jarray_get(arr, i);
		EXPECT_TRUE(jis_number(child));
		int32_t i32(-1);
		EXPECT_EQ(jnumber_get_i32(child, &i32), CONV_OK);
		EXPECT_EQ(i32, i);
	}
}

TEST(TestDOM, StringSimple)
{
	char const data[] = "foo bar. the quick brown\0 fox jumped over the lazy dog.";
	jvalue_ref str1 = manage(jstring_create(data));
	EXPECT_TRUE(jis_string(str1));
	EXPECT_STREQ(jstring_get_fast(str1).m_str, "foo bar. the quick brown");

	jvalue_ref str2 = manage(jstring_create_nocopy(j_str_to_buffer(data, sizeof(data) - 1)));
	EXPECT_TRUE(jis_string(str2));
	raw_buffer buf = jstring_get_fast(str2);
	EXPECT_EQ(buf.m_len, sizeof(data) - 1);
	EXPECT_EQ(string(buf.m_str, buf.m_str + buf.m_len),
	          string(data, data + sizeof(data) - 1));

	EXPECT_TRUE(jstring_equal2(str1, j_str_to_buffer(data, strlen(data))));
	EXPECT_TRUE(jstring_equal2(str2, j_str_to_buffer(data, sizeof(data) - 1)));
}

struct Dealloc
{
	static volatile int free_count;

	static void Free(void *str)
	{
		free(str);
		++free_count;
	}
};
volatile int Dealloc::free_count = 0;

TEST(TestDOM, StringDealloc)
{
	char const str[] = "the quick brown fox jumped over the lazy dog";

	ASSERT_EQ(45, sizeof(str));
	ASSERT_EQ(44, strlen(str));
	raw_buffer srcString = J_CSTR_TO_BUF(str);
	ASSERT_EQ(44, srcString.m_len);

	char *dynstr = (char *)calloc(srcString.m_len + 1, sizeof(char));
	ptrdiff_t dynstrlen = strlen(strncpy(dynstr, srcString.m_str, srcString.m_len));
	ASSERT_EQ(srcString.m_len, dynstrlen);

	jvalue_ref created_string = jstring_create_nocopy_full(j_str_to_buffer(dynstr, dynstrlen), Dealloc::Free);
	jvalue_ref old_ref = created_string;
	EXPECT_EQ(0, Dealloc::free_count);
	j_release(&created_string);
#ifndef NDEBUG
	// we might not compile with DEBUG_FREED_POINTERS even in non-release mode
	EXPECT_TRUE(created_string == (void *)0xdeadbeef || created_string == old_ref);
#else
	EXPECT_TRUE(created_string == old_ref);
#endif
	EXPECT_EQ(1, Dealloc::free_count);
}

TEST(TestDOM, Boolean)
{
	jvalue_ref jval;
	bool val(false);

	jval = manage(jboolean_create(true));
	EXPECT_EQ(CONV_OK, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jboolean_create(false));
	EXPECT_EQ(CONV_OK, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);

	jval = jnull();
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);

#ifndef NDEBUG
	// __attribute__((non_null)) might let the compiler optimize out
	// the check.  this might also break non-gcc compilers
	jval = NULL;
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);
#endif

	jval = manage(jobject_create());
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jobject_create_var(jkeyval(J_CSTR_TO_JVAL("nothing"), jnull()), J_END_OBJ_DECL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jarray_create_var(NULL, J_END_ARRAY_DECL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jarray_create_var(NULL, jnull(), J_END_ARRAY_DECL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jarray_create_var(NULL, jboolean_create(false), J_END_ARRAY_DECL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(J_CSTR_TO_JVAL(""));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);

	jval = manage(J_CSTR_TO_JVAL("false"));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jnumber_create_f64(0));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);

	jval = manage(jnumber_create_unsafe(J_CSTR_TO_BUF("0"), NULL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);

	jval = manage(jnumber_create_unsafe(J_CSTR_TO_BUF("0.0"), NULL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, false);

	jval = manage(jnumber_create_unsafe(J_CSTR_TO_BUF("124"), NULL));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(val, true);

	jval = manage(jnumber_create_i64(1));
	EXPECT_EQ(CONV_NOT_A_BOOLEAN, jboolean_get(jval, &val));
	EXPECT_EQ(true, val);
}
