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

#ifndef TESTJVALUE_H_
#define TESTJVALUE_H_

#include <QTest>
#include <QByteArray>
#include <QObject>
#include <string>
#include <vector>
#include <pbnjson.hpp>

namespace pjson {
namespace testcxx {

class TestJValue : public QObject
{
	Q_OBJECT

public:
	TestJValue();
	virtual ~TestJValue();

private slots:
	void initTestCase(); /// before all tests
	void init(); /// before each test
	void cleanup(); /// after after each test function
	void cleanupTestCase();	/// after all tests

	void testConstructors();

	void testAssignmentOperator();
	void testStdStringComparisonOperator();
	void testBooleanComparisonOperator();
	void testNumberComparisonOperators();

	void testCompareEmptyJValue();
	void testCompareNumbers_data();
	void testCompareNumbers();
	void testCompareBooleans_data();
	void testCompareBooleans();
	void testCompareStrings_data();
	void testCompareStrings();
	void testCompareArrays_data();
	void testCompareArrays();
	void testCompareObjects_data();
	void testCompareObjects();
	void testCompareNumberAndBoolean();
	void testCompareNumberAndString();
	void testCompareNumberAndArray();
	void testCompareNumberAndObject();
	void testCompareBooleanAndString();
	void testCompareBooleanAndArray();
	void testCompareBooleanAndObject();
	void testCompareStringAndArray();
	void testCompareStringAndObject();
	void testCompareArrayAndObject();

	void testBracketAccessorForSimpleTypes();
	void testBracketAccessorForArrays();
	void testBracketAccessorForObjects();

};

}
}

#endif /* TESTJVALUE_H_ */
