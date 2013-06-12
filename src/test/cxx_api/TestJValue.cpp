// @@@LICENSE
//
//      Copyright ©2009-2013 Zenith Electronics LLC, a subsidiary of LG Electronics USA, Inc. All Rights Reserved.
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

#include "TestJValue.h"
#include <QTest>
#include <QtDebug>
#include <QMetaType>
#include <iostream>
#include <cassert>
#include <limits>
#include <execinfo.h>

#include <pbnjson.hpp>
#include <pbnjson.h>

#include "../QBacktrace.h"

Q_DECLARE_METATYPE(pbnjson::JValue);
Q_DECLARE_METATYPE(ConversionResultFlags);
Q_DECLARE_METATYPE(bool);

using namespace std;

namespace pjson {
namespace testcxx {

namespace pj = pbnjson;

TestJValue::TestJValue()
{
}

TestJValue::~TestJValue()
{

}

void TestJValue::initTestCase()
{

}

void TestJValue::init()
{
}

void TestJValue::cleanup()
{
}

void TestJValue::cleanupTestCase()
{
}

// Helper function for converting JValue to string
std::string convertToString(pj::JValue value)
{
	pj::JGenerator serializer;
	pj::JSchemaFragment schema("{}");
	std::string result;
	serializer.toString(value, schema, result);
	return result;
}

// Helper function for creating new data rows
// Note that this only works for objects and arrays (you can't have valid json with just number for example)
QTestData & createNewRow(pj::JValue value1, pj::JValue value2)
{
	std::string string1 = convertToString(value1);
	std::string string2 = convertToString(value2);
	string1.append(" == ");
	string1.append(string2);
	return QTest::newRow(string1.c_str()) << value1 << value2;
}

void TestJValue::testConstructors()
{
	QVERIFY(pj::JValue().isNull());
	QVERIFY(pj::JValue(1).isNumber());
	QVERIFY(pj::JValue("1").isString());
	QVERIFY(pj::JValue(true).isBoolean());
	QVERIFY(pj::Object().isObject());
	QVERIFY(pj::Array().isArray());
}

void TestJValue::testAssignmentOperator()
{
	pj::JValue valueNull = pj::JValue();
	pj::JValue string = pj::JValue("test");
	pj::JValue value = pj::JValue(1);
	value = valueNull;
	QVERIFY(value == valueNull);
	value = string;
	QVERIFY(value == string);
	valueNull = string;
	QVERIFY(valueNull == value);
}

void TestJValue::testStdStringComparisonOperator()
{
	QEXPECT_FAIL("", "JValue does not contain a char * comparison operator, so plain strings will be tried to be matches as booleans", Continue);
	QVERIFY(pj::JValue("t") == "t");
	QVERIFY(pj::JValue("test") == std::string("test"));
	QVERIFY(pj::JValue("test1") != std::string("test"));
	QVERIFY(pj::JValue(1) != std::string("1"));
	QVERIFY(pj::JValue(true) != std::string("true"));
	QVERIFY(pj::Object() != std::string(""));
	QVERIFY(pj::Array() != std::string(""));
}

void TestJValue::testBooleanComparisonOperator()
{
	QVERIFY(pj::JValue(true) == true);
	QVERIFY(pj::JValue(false) == false);
	QVERIFY(pj::JValue(0) != false);
	QVERIFY(pj::JValue("") != false);
	QVERIFY(pj::Object() != false);
	QVERIFY(pj::Array() != false);
}

void TestJValue::testNumberComparisonOperators()
{
	double d(0);
	int64_t int64t(1);
	int32_t int32t(2);

	QVERIFY(pj::JValue(0) == d);
	QVERIFY(pj::JValue(1) == int64t);
	QVERIFY(pj::JValue(2) == int32t);
	QVERIFY(pj::JValue(3) != d);
	QVERIFY(pj::JValue(3) != int64t);
	QVERIFY(pj::JValue(3) != int32t);
	QVERIFY(pj::JValue(false) != d);
	QVERIFY(pj::JValue(true) != int64t);
	QVERIFY(pj::JValue(true) != int32t);
	QVERIFY(pj::JValue("0") != d);
	QVERIFY(pj::JValue("1") != int64t);
	QVERIFY(pj::JValue("2") != int32t);
	QVERIFY(pj::Object() != d);
	QVERIFY(pj::Object() != int64t);
	QVERIFY(pj::Object() != int32t);
	QVERIFY(pj::Array() != d);
	QVERIFY(pj::Array() != int64t);
	QVERIFY(pj::Array() != int32t);
}

void TestJValue::testCompareEmptyJValue()
{
	QVERIFY(pj::JValue() == pj::JValue());
}

void TestJValue::testCompareNumbers_data()
{
	QTest::addColumn<int>("number1");
	QTest::addColumn<int>("number2");
	QTest::addColumn<bool>("expectedResult");

	QTest::newRow("1 == 2") << 1 << 2 << false;
	QTest::newRow("42 == 42") << 42 << 42 << true;
	QTest::newRow("0 == 1000000") << 0 << 1000000 << false;
	QTest::newRow("-1 == 1") << -1 << 1 << false;
}

void TestJValue::testCompareNumbers()
{
	QFETCH(int, number1);
	QFETCH(int, number2);
	QFETCH(bool, expectedResult);

	bool result = pj::JValue(number1) == pj::JValue(number2);
	QCOMPARE(result, expectedResult);
}

void TestJValue::testCompareBooleans_data()
{
	QTest::addColumn<bool>("bool1");
	QTest::addColumn<bool>("bool2");
	QTest::addColumn<bool>("expectedResult");

	QTest::newRow("true == true") << true << true << true;
	QTest::newRow("false == false") << false << false << true;
	QTest::newRow("true == false") << true << false << false;
	QTest::newRow("false == true") << false << true << false;
}

void TestJValue::testCompareBooleans()
{
	QFETCH(bool, bool1);
	QFETCH(bool, bool2);
	QFETCH(bool, expectedResult);

	bool result = pj::JValue(bool1) == pj::JValue(bool2);
	QCOMPARE(result, expectedResult);
}

void TestJValue::testCompareStrings_data()
{
	QTest::addColumn<QString>("string1");
	QTest::addColumn<QString>("string2");
	QTest::addColumn<bool>("expectedResult");

	QTest::newRow("'first' == 'second'") << "first" << "second" << false;
	QTest::newRow("empty == 'second'") << QString() << "second" << false;
	QTest::newRow("'first' == 'first'") << "first" << "first" << true;
}

void TestJValue::testCompareStrings()
{
	QFETCH(QString, string1);
	QFETCH(QString, string2);
	QFETCH(bool, expectedResult);

	bool result = pj::JValue(string1.toStdString()) == pj::JValue(string2.toStdString());
	QCOMPARE(result, expectedResult);
}

void TestJValue::testCompareArrays_data()
{
	QTest::addColumn<pj::JValue>("array1");
	QTest::addColumn<pj::JValue>("array2");
	QTest::addColumn<bool>("expectedResult");

	createNewRow(pj::Array(), pj::Array()) << true;
	createNewRow(
		(pj::Array() << "a1"),
		(pj::Array() << "a1")) <<
		true;
	createNewRow(
		(pj::Array() << "a1"),
		(pj::Array() << "a2")) <<
		false;
	createNewRow(
		(pj::Array() << "a1"),
		(pj::Array() << "a1" << "a1")) <<
		false;
	createNewRow(
		(pj::Array() << "a2" << "a2"),
		(pj::Array() << "a1"<< "a1")) <<
		false;
	createNewRow(
		(pj::Array() << "a1" << "a1"),
		(pj::Array() << "a1" << "a1")) <<
		true;
	createNewRow(
		(pj::Array() << "a1" << "a2"),
		(pj::Array() << "a1" << "a1")) <<
		false;
	createNewRow(
		(pj::Array() << "a2" << "a1"),
		(pj::Array() << "a1" << "a1")) <<
		false;
	createNewRow(
		(pj::Array() << 1 << 2),
		(pj::Array() << 2 << 1)) <<
		false;
	createNewRow(
		(pj::Array() << 1 << 2),
		(pj::Array() << 1 << 2)) <<
		true;
	createNewRow(
		(pj::Array() << true << false),
		(pj::Array() << false << true)) <<
		false;
	createNewRow(
		(pj::Array() << true << false),
		(pj::Array() << true << false)) <<
		true;
	createNewRow(
		(pj::Array() << pj::Object()),
		(pj::Array() << pj::Object())) <<
		true;
	createNewRow(
		(pj::Array() << (pj::Object() << pj::JValue::KeyValue("id", 1))),
		(pj::Array() << (pj::Object() << pj::JValue::KeyValue("id", 2)))) <<
		false;
	createNewRow(
		(pj::Array() << (pj::Object() << pj::JValue::KeyValue("id", 1))),
		(pj::Array() << (pj::Object() << pj::JValue::KeyValue("id", 1)))) <<
		true;
	createNewRow(
		(pj::Array() << (pj::Object() << pj::JValue::KeyValue("id", 1))),
		(pj::Array() << (pj::Object() << pj::JValue::KeyValue("id", 1)) << (pj::Object() << pj::JValue::KeyValue("name", "John")))) <<
		false;
}

void TestJValue::testCompareArrays()
{
	QFETCH(pj::JValue, array1);
	QFETCH(pj::JValue, array2);
	QFETCH(bool, expectedResult);

	bool result = array1 == array2;
	QCOMPARE(result, expectedResult);
}

void TestJValue::testCompareObjects_data()
{
	QTest::addColumn<pj::JValue>("object1");
	QTest::addColumn<pj::JValue>("object2");
	QTest::addColumn<bool>("expectedResult");
	QTest::addColumn<bool>("expectedFail"); // TODO: after object comparison is fixed, remove this.

	createNewRow(pj::Object(), pj::Object()) << true << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1)),
		(pj::Object() << pj::JValue::KeyValue("id", 2))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1)),
		(pj::Object() << pj::JValue::KeyValue("id1", 1))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1)),
		(pj::Object() << pj::JValue::KeyValue("id", 1))) <<
		true << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1)),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John"))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John")),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John"))) <<
		true << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name1", "John")),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John"))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John")),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John1"))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John") << pj::JValue::KeyValue("bool", false)),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John") << pj::JValue::KeyValue("bool", true))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John") << pj::JValue::KeyValue("bool", true)),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John") << pj::JValue::KeyValue("bool", true))) <<
		true << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John") << pj::JValue::KeyValue("bool", false)),
		(pj::Object() << pj::JValue::KeyValue("id", 1) << pj::JValue::KeyValue("name", "John") << pj::JValue::KeyValue("bool", pj::JValue()))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("array", pj::Array())),
		(pj::Object() << pj::JValue::KeyValue("array", pj::Array()))) <<
		true << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("array", (pj::Array() << 1))),
		(pj::Object() << pj::JValue::KeyValue("array", (pj::Array() << 2)))) <<
		false << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("array", (pj::Array() << 1))),
		(pj::Object() << pj::JValue::KeyValue("array", (pj::Array() << 1)))) <<
		true << false;
	createNewRow(
		(pj::Object() << pj::JValue::KeyValue("array", (pj::Array() << 1))),
		(pj::Object() << pj::JValue::KeyValue("array", (pj::Array() << 1)) << pj::JValue::KeyValue("array2", (pj::Array() << 1)))) <<
		false << false;
}

void TestJValue::testCompareObjects()
{
	QFETCH(pj::JValue, object1);
	QFETCH(pj::JValue, object2);
	QFETCH(bool, expectedResult);
	QFETCH(bool, expectedFail);

	if (expectedFail) {
		QEXPECT_FAIL("", "Object comparison doesn't compare for object size's", Continue);
	}

	bool result = object1 == object2;
	QCOMPARE(result, expectedResult);
}

void TestJValue::testCompareNumberAndBoolean()
{
	QVERIFY(pj::JValue(1) != pj::JValue(true));
	QVERIFY(pj::JValue(0) != pj::JValue(false));
	QVERIFY(pj::JValue(false) != pj::JValue(0));
}

void TestJValue::testCompareNumberAndString()
{
	QVERIFY(pj::JValue(1) != pj::JValue("1"));
	QVERIFY(pj::JValue(3) != pj::JValue("3"));
	QVERIFY(pj::JValue("0") != pj::JValue(0));
}

void TestJValue::testCompareNumberAndArray()
{
	QVERIFY(pj::JValue(0) != pj::Array());
	QVERIFY(pj::JValue(0) != (pj::Array() << 0));
	QVERIFY((pj::Array() << 0) != pj::JValue(0));
}

void TestJValue::testCompareNumberAndObject()
{
	QVERIFY(pj::JValue(0) != pj::Object());
	QVERIFY(pj::JValue(0) != (pj::Object() << pj::JValue::KeyValue("number",0)));
	QVERIFY((pj::Object() << pj::JValue::KeyValue("number",0)) != pj::JValue(0));
}

void TestJValue::testCompareBooleanAndString()
{
	QVERIFY(pj::JValue(true) != pj::JValue("true"));
	QVERIFY(pj::JValue(false) != pj::JValue("false"));
	QVERIFY(pj::JValue("false") != pj::JValue(false));
}

void TestJValue::testCompareBooleanAndArray()
{
	QVERIFY(pj::JValue(false) != pj::Array());
	QVERIFY(pj::JValue(true) != (pj::Array() << true));
	QVERIFY((pj::Array() << true) != pj::JValue(true));
}

void TestJValue::testCompareBooleanAndObject()
{
	QVERIFY(pj::JValue(false) != pj::Object());
	QVERIFY(pj::JValue(true) != (pj::Object() << pj::JValue::KeyValue("boolean",true)));
	QVERIFY((pj::Object() << pj::JValue::KeyValue("boolean",true)) != pj::JValue(true));
}

void TestJValue::testCompareStringAndArray()
{
	QVERIFY(pj::JValue("") != pj::Array());
	QVERIFY(pj::JValue("test") != (pj::Array() << "test"));
	QVERIFY((pj::Array() << "test") != pj::JValue("test"));
}

void TestJValue::testCompareStringAndObject()
{
	QVERIFY(pj::JValue("") != pj::Object());
	QVERIFY(pj::JValue("test") != (pj::Object() << pj::JValue::KeyValue("string","test")));
	QVERIFY((pj::Object() << pj::JValue::KeyValue("string","test")) != pj::JValue("test"));
}

void TestJValue::testCompareArrayAndObject()
{
	QVERIFY(pj::Array() != pj::Object());
	QVERIFY((pj::Array() << 1) != (pj::Object() << pj::JValue::KeyValue("number",1)));
	QVERIFY((pj::Object() << pj::JValue::KeyValue("string","test")) != (pj::Array() << "test"));
}

void TestJValue::testBracketAccessorForSimpleTypes()
{
	pj::JValue valueNull = pj::JValue();
	pj::JValue number = pj::JValue(1);
	pj::JValue boolean = pj::JValue(true);
	pj::JValue string = pj::JValue("test");

	QVERIFY(valueNull[0] == pj::JValue());
	QVERIFY(number[0] == pj::JValue());
	QVERIFY(boolean[0] == pj::JValue());
	QVERIFY(string[0] == pj::JValue());
}

void TestJValue::testBracketAccessorForArrays()
{
	pj::JValue array = pj::Array();
	array.append("1");
	QVERIFY(array[0] == std::string("1"));
	array[0] = 2;
	QEXPECT_FAIL("", "Assigning to array with bracket operator is not working", Continue);
	QVERIFY(array[0] == 2);
	array.append(3);
	QVERIFY(array[1] == 3);
	QVERIFY(array[2] == pj::JValue());
	array.append(pj::JValue());
	QVERIFY(array[2] == pj::JValue());
	QVERIFY(array["key"] == pj::JValue());
}

void TestJValue::testBracketAccessorForObjects()
{
	pj::JValue object = pj::Object();
	object.put("key1", "1");
	QVERIFY(object["key1"] == std::string("1"));
	object["key1"] == 2;
	QEXPECT_FAIL("", "Assigning to object with bracket operator is not working", Continue);
	QVERIFY(object["key1"] == 2);
	object.put("key2", 3);
	QVERIFY(object["key2"] == 3);
	QVERIFY(object["key3"] == pj::JValue());
	object.put("key3", pj::JValue());
	QVERIFY(object["key3"] == pj::JValue());
	QVERIFY(object[0] == pj::JValue());
}

}
}

QTEST_APPLESS_MAIN(pjson::testcxx::TestJValue);
