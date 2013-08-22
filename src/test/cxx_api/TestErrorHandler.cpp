// @@@LICENSE
//
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
#include <pbnjson.hpp>

using namespace pbnjson;
using namespace std;

struct ErrorHandlerCounter : pbnjson::JErrorHandler
{
	int syntaxCounter;
	int schemaCounter;
	int miscCounter;
	int badObjectCounter;
	int badArrayCounter;
	int badStringCounter;
	int badNumberCounter;
	int badBooleanCounter;
	int badNullCounter;
	int parseFailedCounter;
	string errorDescription;

	ErrorHandlerCounter()
		: syntaxCounter(0)
		, schemaCounter(0)
		, miscCounter(0)
		, badObjectCounter(0)
		, badArrayCounter(0)
		, badStringCounter(0)
		, badNumberCounter(0)
		, badBooleanCounter(0)
		, badNullCounter(0)
		, parseFailedCounter(0)
	{
	}

	void syntax(pbnjson::JParser *, SyntaxError, const std::string &)
	{
		++syntaxCounter;
	}

	virtual void schema(pbnjson::JParser *, SchemaError, const std::string &)
	{
		++schemaCounter;
	}

	virtual void misc(pbnjson::JParser *, const std::string &)
	{
		++miscCounter;
		errorDescription = reason;
	}

	virtual void badObject(pbnjson::JParser *, BadObject)
	{
		++badObjectCounter;
	}

	virtual void badArray(pbnjson::JParser *, BadArray)
	{
		++badArrayCounter;
	}

	virtual void badString(pbnjson::JParser *, const std::string &)
	{
		++badStringCounter;
	}

	virtual void badNumber(pbnjson::JParser *, const std::string &)
	{
		++badNumberCounter;
	}

	virtual void badBoolean(pbnjson::JParser *)
	{
		++badBooleanCounter;
	}

	virtual void badNull(pbnjson::JParser *)
	{
		++badNullCounter;
	}

	virtual void parseFailed(pbnjson::JParser *, const std::string &)
	{
		++parseFailedCounter;
	}
};

typedef shared_ptr<ErrorHandlerCounter> ErrorHandlerCounterPtr;

ErrorHandlerCounterPtr errorHandlerCounterPtr;

class JErrorHandlerTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		errorHandlerCounterPtr = make_shared<ErrorHandlerCounter>();
	}

	virtual void TearDown()
	{
		errorHandlerCounterPtr.reset();
	}
};

TEST_F(JErrorHandlerTest, misc)
{
	pbnjson::JDomParser parser;
	string faultyJSON = "{ \"keyword\" :  }";

	ASSERT_FALSE(parser.parse(faultyJSON, JSchema::AllSchema(), errorHandlerCounterPtr.get()));

	EXPECT_EQ(1, errorHandlerCounterPtr->parseFailedCounter);
	EXPECT_EQ(1, errorHandlerCounterPtr->miscCounter);
	EXPECT_NE(string::npos, errorHandlerCounterPtr->errorDescription.find("unallowed token at this point"));
}

