// @@@LICENSE
//
//      Copyright (c) 2013 LG Electronics, Inc.
//      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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
#include <pbnjson.hpp>
#include <algorithm>

using namespace std;
using namespace pbnjson;

static ssize_t GetObjectSize(const pbnjson::JValue& value)
{
	ssize_t result = 0;
	for (pbnjson::JValue::ObjectConstIterator i = value.begin(); i != value.end(); ++i)
		++result;
	return result;
}

TEST(TestDOM, ParserObjectSimple)
{
	JDomParser parser(nullptr);
	JSchemaFragment schema("{}");

	ASSERT_TRUE(parser.parse("{}", schema));

	JValue parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(parsed.begin() == parsed.end());
	EXPECT_FALSE(parsed.begin() != parsed.end());
	EXPECT_EQ(GetObjectSize(parsed), ssize_t{0});
}

TEST(TestDOM, ParserObjectComplex)
{
	JDomParser parser(NULL);
	JSchemaFragment schema("{}");

	std::string dom1Str = "{\"key1\" : null, \"key2\" : \"str\", \"key3\" : 506 }";
	ASSERT_TRUE(parser.parse(dom1Str, schema));
	JValue parsed = parser.getDom();
	EXPECT_EQ(GetObjectSize(parsed), ssize_t{3});

	auto val1 = JValue{parsed["key1"]};
	EXPECT_TRUE(val1.isNull());

	auto val2 = JValue{parsed["key2"]};
	EXPECT_TRUE(val2.isString());
	EXPECT_EQ(val2.asString(), string{"str"});

	auto val3 = JValue{parsed["key3"]};
	EXPECT_TRUE(val3.isNumber());
	EXPECT_EQ(val3.asNumber<std::string>(), string{"506"});
	EXPECT_EQ(val3.asNumber<int64_t>(), int64_t{506});
}

TEST(TestDOM, ParserObjectComplex2)
{
	JDomParser parser(NULL);
	JSchemaFragment schema("{}");

	std::string dom2Str =
		"{"
			"\"key1\": {"
				"\"key1\": {"
						"\"foo\": [\"bar\", 5, null],"
						"\"bar\": false,"
						"\"test\": \"abcd\""
				"}"
			"}"
		"}"
	;

	ASSERT_TRUE(parser.parse(dom2Str, schema));

	JValue parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_EQ(GetObjectSize(parsed), ssize_t{1});

	EXPECT_TRUE(parsed["key1"].isObject());
	EXPECT_EQ(GetObjectSize(parsed["key1"]), ssize_t{1});

	EXPECT_TRUE(parsed["key1"]["key1"].isObject());
	EXPECT_EQ(GetObjectSize(parsed["key1"]["key1"]), ssize_t{3});

	EXPECT_TRUE(parsed["key1"]["key1"]["foo"].isArray());
	EXPECT_EQ(parsed["key1"]["key1"]["foo"].arraySize(), ssize_t{3});

	// the (ssize_t) cast is only necessary for OSX 10.5 which uses an old compiler
	EXPECT_TRUE(parsed["key1"]["key1"]["foo"][0].isString());
	EXPECT_EQ(parsed["key1"]["key1"]["foo"][0].asString(), std::string{"bar"});

	EXPECT_TRUE(parsed["key1"]["key1"]["foo"][1].isNumber());
	EXPECT_EQ(parsed["key1"]["key1"]["foo"][1].asNumber<int32_t>(), 5);

	EXPECT_TRUE(parsed["key1"]["key1"]["foo"][2].isNull());

	EXPECT_TRUE(parsed["key1"]["key1"]["bar"].isBoolean());
	EXPECT_EQ(parsed["key1"]["key1"]["bar"].asBool(), false);

	EXPECT_TRUE(parsed["key1"]["key1"]["test"].isString());
	EXPECT_EQ(parsed["key1"]["key1"]["test"].asString(), std::string{"abcd"});
}

TEST(TestDOM, ParserObjectCommaMemLeak)
{
	JDomParser parser(NULL);
	JSchemaFragment schema("{}");

	// this test was causing a memory leak (otherwise it is a duplicate of above)
	// it fails because of the null (turns into ,null,], - the ',' after the null is illegal).
	std::string dom2Str =
		"{"
			"\"key1\": {"
				"\"key1\": {"
						"\"foo\": [\"bar\", 5, null,],"
						"\"bar\": false,"
						"\"test\": \"abcd\""
				"}"
			"}"
		"}"
	;

	ASSERT_FALSE(parser.parse(dom2Str, schema));
}

TEST(TestDOM, ObjectSimple)
{
	JValue simpleObject = Object();
	JSchemaFragment schema("{}");
	JGenerator generator(NULL);

	simpleObject.put("abc", "def");
	EXPECT_TRUE(simpleObject.hasKey("abc"));
	EXPECT_TRUE(simpleObject["abc"].isString());
	EXPECT_EQ(simpleObject["abc"].asString(), std::string{"def"});

	simpleObject.put("def", NumericString("5463"));
	EXPECT_TRUE(simpleObject.hasKey("def"));
	EXPECT_TRUE(simpleObject["def"].isNumber());
	EXPECT_EQ(simpleObject["def"].asNumber<int32_t>(), 5463);

	string simpleObjectAsStr;
	ASSERT_TRUE(generator.toString(simpleObject, schema, simpleObjectAsStr));
	EXPECT_EQ(simpleObjectAsStr, string("{\"abc\":\"def\",\"def\":5463}"));

	EXPECT_TRUE(simpleObject.isObject());
}

TEST(TestDOM, ObjectIterator)
{
	const char *inputStr = "{\"a\":{\"b\":\"c\", \"d\":5}}";
	JDomParser parser;
	JSchemaFragment schema("{}");

	ASSERT_TRUE(parser.parse(inputStr, schema));
	JValue parsed = parser.getDom();

	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(parsed.hasKey("a"));
	EXPECT_TRUE(parsed["a"].isObject());
	EXPECT_TRUE(parsed["a"].hasKey("b"));
	EXPECT_TRUE(parsed["a"].hasKey("d"));
	EXPECT_TRUE(parsed["a"]["b"].isString());
	EXPECT_EQ(parsed["a"]["b"].asString(), string("c"));
	EXPECT_TRUE(parsed["a"]["d"].isNumber());
	EXPECT_EQ(parsed["a"]["d"].asNumber<int>(), 5);

	JValue::ObjectIterator i = parsed.begin();
	ASSERT_TRUE(i != parsed.end());
	EXPECT_TRUE((*i).first.isString());
	EXPECT_EQ((*i).first.asString(), string("a"));
	EXPECT_TRUE((*i).second.isObject());

	i = (*i).second.begin();
	EXPECT_TRUE(i == parsed["a"].begin());
	ASSERT_TRUE(i != parsed["a"].end());
	EXPECT_TRUE((*i).first.isString());
	EXPECT_EQ((*i).first.asString(), string("b"));
	EXPECT_TRUE((*i).second.isString());
	EXPECT_EQ((*i).second.asString(), string("c"));

	i++;
	EXPECT_TRUE((*i).first.isString());
	EXPECT_EQ((*i).first.asString(), string("d"));
	EXPECT_TRUE((*i).second.isNumber());
	EXPECT_EQ((*i).second.asNumber<int>(), 5);
}

// sanity check that assumptions about limits of double storage
// are correct
static const int64_t maxDblPrecision = 0x1FFFFFFFFFFFFFLL;
static const int64_t minDblPrecision = -0x1FFFFFFFFFFFFFLL;
static const int64_t positiveOutsideDblPrecision = maxDblPrecision + 2; // +1 doesn't work because it's the same (it truncates a 0)
static const int64_t negativeOutsideDblPrecision = minDblPrecision - 2; // +1 doesn't work because it's the same (it truncates a 0)
static const int64_t maxInt32 = std::numeric_limits<int32_t>::max();
static const int64_t minInt32 = std::numeric_limits<int32_t>::min();
static const char *weirdString = "long and complicated \" string ' with \" $*U@*(&#(@*&";
static const char *veryLargeNumber = "645458489754321564894654151561684894456464513215648946543132189489461321684.2345646544e509";

TEST(TestDOM, DoubleCheck)
{
	double check = (double)maxDblPrecision;
	EXPECT_EQ((int64_t)check, maxDblPrecision);

	check = (double)minDblPrecision;
	EXPECT_EQ((int64_t)check, minDblPrecision);

	check = (double)(positiveOutsideDblPrecision);
	EXPECT_NE((int64_t)check, positiveOutsideDblPrecision);

	check = (double)(negativeOutsideDblPrecision);
	EXPECT_NE((int64_t)check, negativeOutsideDblPrecision);
}

TEST(TestDOM, ObjectComplex)
{
	EXPECT_TRUE(std::numeric_limits<double>::has_quiet_NaN);
	EXPECT_TRUE(std::numeric_limits<double>::has_signaling_NaN);

	// unfortunately, C++ doesn't allow us to use J_CSTR_TO_JVAL which is what I would use
	// for string literals under C.
	JValue obj1Embedded = Object();
	obj1Embedded.put("num_1", NumericString("64.234"));
	obj1Embedded.put("num_2", NumericString(veryLargeNumber));

	JValue complexObject = Object();
	complexObject.put("bool1", true);
	complexObject.put("bool2", false);
	complexObject.put("numi32_1", 0),
	complexObject.put("numi32_2", -50);
	complexObject.put("numi32_3", 12345323);
	complexObject.put("numi64_1", maxInt32 + 1);
	complexObject.put("numi64_2", minInt32 - 1);
	complexObject.put("numi64_3", 0);
	complexObject.put("numi64_4", maxDblPrecision);
	complexObject.put("numi64_5", minDblPrecision);
	complexObject.put("numi64_6", positiveOutsideDblPrecision);
	complexObject.put("numi64_7", negativeOutsideDblPrecision);
	complexObject.put("numf64_1", 0.45642156489);
	complexObject.put("numf64_2", -54897864.14);
	complexObject.put("numf64_3", -54897864);
	complexObject.put("numf64_4", std::numeric_limits<double>::infinity());
	complexObject.put("numf64_5", -std::numeric_limits<double>::infinity());
	complexObject.put("numf64_6", -std::numeric_limits<double>::quiet_NaN());
	complexObject.put("str1", JValue());
	complexObject.put("str2", JValue());
	complexObject.put("str3", "");
	complexObject.put("str4", "foo");
	complexObject.put("str5", weirdString);
	complexObject.put("obj1", obj1Embedded);

	EXPECT_EQ(complexObject["bool1"].asBool(), true);
	EXPECT_EQ(complexObject["bool2"].asBool(), false);

	EXPECT_EQ(complexObject["numi32_1"].asNumber<int32_t>(), 0);
	EXPECT_EQ(complexObject["numi32_2"].asNumber<int32_t>(), -50);
	EXPECT_EQ(complexObject["numi32_3"].asNumber<int32_t>(), 12345323);
	EXPECT_EQ(complexObject["numi64_1"].asNumber<int64_t>(), maxInt32 + 1);
	int32_t i32(0);
	EXPECT_EQ(CONV_POSITIVE_OVERFLOW, complexObject["numi64_1"].asNumber(i32));
	EXPECT_EQ(complexObject["numi64_2"].asNumber<int64_t>(), minInt32 - 1);
	EXPECT_EQ(CONV_NEGATIVE_OVERFLOW, complexObject["numi64_2"].asNumber(i32));
	EXPECT_EQ(complexObject["numi64_3"].asNumber<int64_t>(), 0);
	EXPECT_EQ(complexObject["numi64_4"].asNumber<int64_t>(), maxDblPrecision);
	EXPECT_EQ(complexObject["numi64_4"].asNumber<double>(), (double)maxDblPrecision);
	EXPECT_EQ(complexObject["numi64_5"].asNumber<int64_t>(), minDblPrecision);
	EXPECT_EQ(complexObject["numi64_5"].asNumber<double>(), (double)minDblPrecision);
	EXPECT_EQ(complexObject["numi64_6"].asNumber<int64_t>(), positiveOutsideDblPrecision);
	double dbl(0.0);
	EXPECT_EQ(CONV_PRECISION_LOSS, complexObject["numi64_6"].asNumber<double>(dbl));
	EXPECT_EQ(complexObject["numi64_7"].asNumber<int64_t>(), negativeOutsideDblPrecision);
	EXPECT_EQ(CONV_PRECISION_LOSS, complexObject["numi64_7"].asNumber<double>(dbl));
	EXPECT_EQ(complexObject["numf64_1"].asNumber<double>(), 0.45642156489);
	int64_t i64(0);
	EXPECT_EQ(CONV_PRECISION_LOSS, complexObject["numf64_1"].asNumber<int64_t>(i64));
	EXPECT_EQ(complexObject["numf64_2"].asNumber<double>(), -54897864.14);
	EXPECT_EQ(CONV_PRECISION_LOSS, complexObject["numf64_2"].asNumber<int64_t>(i64));
	EXPECT_EQ(complexObject["numf64_3"].asNumber<double>(), -54897864);
	EXPECT_EQ(complexObject["numf64_3"].asNumber<int64_t>(), -54897864);

	EXPECT_TRUE(complexObject["numf64_4"].isNull()); // + inf
	EXPECT_TRUE(complexObject["numf64_5"].isNull()); // - inf
	EXPECT_TRUE(complexObject["numf64_6"].isNull()); // NaN

	EXPECT_TRUE(complexObject["str1"].isNull());
	EXPECT_TRUE(complexObject["str2"].isNull());

	EXPECT_EQ(complexObject["str3"].asString(), string{""});
	EXPECT_EQ(complexObject["str4"].asString(), string{"foo"}); // NaN
	EXPECT_EQ(complexObject["str5"].asString(), weirdString); // NaN

	JValue obj1 = complexObject["obj1"];
	EXPECT_EQ(obj1["num_1"].asNumber<int64_t>(i64), CONV_PRECISION_LOSS);
	EXPECT_EQ(obj1["num_1"].asNumber<double>(), 64.234);
	EXPECT_EQ(obj1["num_2"].asNumber<int64_t>(i64), CONV_POSITIVE_OVERFLOW | CONV_PRECISION_LOSS);
	EXPECT_EQ(obj1["num_2"].asNumber<double>(dbl), CONV_POSITIVE_OVERFLOW);
	EXPECT_EQ(obj1["num_2"].asNumber<string>(), string{veryLargeNumber});
}

TEST(TestDOM, ObjectPut)
{
	JValue obj = Object();
	obj.put("abc", 5);
	obj.put("abc", "def");

	EXPECT_TRUE(obj["abc"].isString());
	EXPECT_EQ(obj["abc"].asString(), string("def"));
}

TEST(TestDOM, ArraySimple)
{
	JValue simple_arr = Array();

	simple_arr.put(4, true);
	EXPECT_EQ(simple_arr.arraySize(), 5);

	simple_arr.put(1, NumericString("1"));
	EXPECT_EQ(simple_arr.arraySize(), 5);

	simple_arr.put(2, 2);
	EXPECT_EQ(simple_arr.arraySize(), 5);

	simple_arr.put(3, false);
	EXPECT_EQ(simple_arr.arraySize(), 5);

	simple_arr.put(5, JValue());
	EXPECT_EQ(simple_arr.arraySize(), 6);

	simple_arr.put(6, "");
	EXPECT_EQ(simple_arr.arraySize(), 7);

	EXPECT_EQ(simple_arr[0], JValue());
	simple_arr.put(0, "index 0");
	EXPECT_EQ(simple_arr.arraySize(), 7);

	simple_arr.put(7, 7.0);
	EXPECT_EQ(simple_arr.arraySize(), 8);

	EXPECT_TRUE(simple_arr[0].isString());
	EXPECT_EQ(simple_arr[0].asString(), string{"index 0"});

	EXPECT_TRUE(simple_arr[1].isNumber());
	EXPECT_EQ(simple_arr[1].asNumber<string>(), string{"1"});
	EXPECT_EQ(simple_arr[1].asNumber<int32_t>(), 1);

	EXPECT_EQ(simple_arr[2].asNumber<int32_t>(), 2);

	EXPECT_EQ(simple_arr[3].asBool(), false);
	EXPECT_EQ(simple_arr[4].asBool(), true);

	EXPECT_TRUE(simple_arr[5].isNull());
	EXPECT_EQ(simple_arr[5], JValue());

	EXPECT_TRUE(simple_arr[6].isString());
	EXPECT_EQ(simple_arr[6].asString(), string{""});

	EXPECT_EQ(simple_arr[7].asNumber<int32_t>(), 7);
}

TEST(TestDOM, ArrayComplex)
{
	JValue arr = Array();
	ASSERT_TRUE(arr.isArray());

	for (int32_t i = 0; i < 100; i++)
	{
		arr.append(i);
		EXPECT_EQ(arr.arraySize(), i + 1);
		EXPECT_EQ(arr[i].asNumber<int32_t>(), i);
	}

	for (int32_t i = 0; i < 100; i++)
	{
		JValue child = arr[i];
		EXPECT_TRUE(child.isNumber());
		int32_t num(-1);
		EXPECT_EQ(child.asNumber<int32_t>(num), CONV_OK);
		EXPECT_EQ(child.asNumber<int32_t>(), i);
	}
}

TEST(TestDOM, StringSimple)
{
	char const str1[] = "foo bar. the quick brown\0 fox jumped over the lazy dog.";
	JValue val1(str1);
	ASSERT_TRUE(val1.isString());
	EXPECT_EQ(val1.asString(), string{str1});

	string full{str1, sizeof(str1) - 1};
	ASSERT_TRUE(full != str1);
	JValue val2{full};
	ASSERT_TRUE(val2.isString());
	EXPECT_EQ(val2.asString(), full);
}

TEST(TestDOM, Boolean)
{
	bool v;

	EXPECT_EQ(JValue(true).asBool(v), CONV_OK);
	EXPECT_EQ(JValue(true).asBool(), true);

	EXPECT_EQ(JValue(false).asBool(v), CONV_OK);
	EXPECT_EQ(JValue(false).asBool(), false);

	EXPECT_EQ(JValue().asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue().asBool(), false);

	JValue empty_obj = Object();
	EXPECT_EQ(empty_obj.asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(empty_obj.asBool(), true);

	JValue non_empty_obj = Object() << JValue::KeyValue("nothing", JValue());
	EXPECT_EQ(non_empty_obj.asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(non_empty_obj.asBool(), true);

	JValue arr = Array();
	EXPECT_EQ(arr.asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(arr.asBool(), true);

	JValue non_empty_arr = Array() << JValue();
	EXPECT_EQ(non_empty_arr.asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(non_empty_arr.asBool(), true);

	JValue arr_false = Array() << false;
	EXPECT_EQ(arr_false.asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(arr_false.asBool(), true);

	EXPECT_EQ(JValue("").asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue("").asBool(), false);

	EXPECT_EQ(JValue("false").asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue("false").asBool(), true);

	EXPECT_EQ(JValue(int64_t{0}).asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue(int64_t{0}).asBool(), false);

	EXPECT_EQ(JValue(0.0).asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue(0.0).asBool(), false);

	EXPECT_EQ(JValue(NumericString("0")).asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue(NumericString("0")).asBool(), false);

	EXPECT_EQ(JValue(NumericString("0.0")).asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue(NumericString("0.0")).asBool(), false);

	EXPECT_EQ(JValue(NumericString("124")).asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue(NumericString("124")).asBool(), true);

	EXPECT_EQ(JValue(int64_t{1}).asBool(v), CONV_NOT_A_BOOLEAN);
	EXPECT_EQ(JValue(int64_t{1}).asBool(), true);
}
