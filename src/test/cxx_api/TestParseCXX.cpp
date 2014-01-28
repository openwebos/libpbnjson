// @@@LICENSE
//
//      Copyright (c) 2014 LG Electronics, Inc.
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
#include <fstream>
#include <vector>
#include <string>

using namespace pbnjson;

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

struct SAXCallbacks : public JParser
{
	SAXCallbacks()
		: JParser(NULL)
		, jsonObjectOpenCount(0)
		, jsonObjectCloseCount(0)
		, jsonArrayOpenCount(0)
		, jsonArrayCloseCount(0)
		, jsonNullCount(0)
	{}

	int jsonObjectOpenCount;
	int jsonObjectCloseCount;
	int jsonArrayOpenCount;
	int jsonArrayCloseCount;
	int jsonNullCount;
	std::vector<std::string> jsonObjectKeyStorage;
	std::vector<std::string> jsonStringStorage;
	std::vector<std::string> jsonNumberStringStorage;
	std::vector<int64_t> jsonNumberInt64Storage;
	std::vector<double> jsonNumberDoubleStorage;
	std::vector<bool> jsonBooleanStorage;

	bool jsonObjectOpen() {
		jsonObjectOpenCount++;
		return true;
	}
	bool jsonObjectKey(const std::string& key) {
		jsonObjectKeyStorage.push_back(key);
		return true;
	}
	bool jsonObjectClose() {
		jsonObjectCloseCount++;
		return true;
	}
	bool jsonArrayOpen() {
		jsonArrayOpenCount++;
		return true;
	}
	bool jsonArrayClose() {
		jsonArrayCloseCount++;
		return true;
	}
	bool jsonString(const std::string& s) {
		jsonStringStorage.push_back(s);
		return true;
	}
	bool jsonNumber(const std::string& n) {
		jsonNumberStringStorage.push_back(n);
		return true;
	}
	bool jsonNumber(int64_t number) {
		jsonNumberInt64Storage.push_back(number);
		return true;
	}
	bool jsonNumber(double &number, ConversionResultFlags asFloat) {
		jsonNumberDoubleStorage.push_back(number);
		return true;
	}
	bool jsonBoolean(bool truth) {
		jsonBooleanStorage.push_back(truth);
		return true;
	}
	bool jsonNull() {
		jsonNullCount++;
		return true;
	}

	NumberType conversionToUse() const {return JNUM_CONV_NATIVE;}
};

template<class T>
std::vector<T> MultVector(const std::vector<T>& vec, int times) {
	std::vector<T> newVec;
	for(int i = 0 ; i < times ; i++) {
		std::copy(vec.begin(),vec.end(),back_inserter(newVec));
	}
	return newVec;
}

TEST(TestParse, saxparser)
{
	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	JSchemaFile schema("../schemas/parse/test_stream_parser.schema");

	for (int j = 0 ; j < 2 ; j++) {
		SAXCallbacks parser;
		for (int k = 1 ; k < 3 ; k++) {
			ASSERT_TRUE(parser.begin(schema));
			for (std::string::const_iterator i = json_str.begin() ; i != json_str.end() ; ++i) {
				ASSERT_TRUE(parser.feed(&(*i), 1));
			}
			ASSERT_TRUE(parser.end());

			EXPECT_EQ(k*1, parser.jsonObjectOpenCount);
			EXPECT_EQ(k*1, parser.jsonObjectCloseCount);
			EXPECT_EQ(k*1, parser.jsonArrayOpenCount);
			EXPECT_EQ(k*1, parser.jsonArrayCloseCount);
			EXPECT_EQ(k*1, parser.jsonNullCount);
			EXPECT_EQ(MultVector(std::vector<std::string>({"null", "bool", "number", "string", "array"}), k), parser.jsonObjectKeyStorage);
			EXPECT_EQ(MultVector(std::vector<std::string>({"asd", "qwerty"}), k), parser.jsonStringStorage);
			EXPECT_EQ(MultVector(std::vector<std::string>(), k), parser.jsonNumberStringStorage);
			EXPECT_EQ(MultVector(std::vector<int64_t>({2}), k), parser.jsonNumberInt64Storage);
			EXPECT_EQ(MultVector(std::vector<double>({1.1}), k), parser.jsonNumberDoubleStorage);
			EXPECT_EQ(MultVector(std::vector<bool>({true}), k), parser.jsonBooleanStorage);
		}
	}
}

TEST(TestParse, domparser)
{
	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	JSchemaFile schema("../schemas/parse/test_stream_parser.schema");

	JValue jval;
	for (int  j = 0 ; j < 2 ; j++) {
		JDomParser parser(NULL);
		for (int k = 0 ; k < 2 ; k++)  {
			ASSERT_TRUE(parser.begin(schema));
			for (std::string::const_iterator i = json_str.begin() ; i != json_str.end() ; ++i) {
				ASSERT_TRUE(parser.feed(&(*i), 1));
			}
			ASSERT_TRUE(parser.end());
		}
		jval = parser.getDom();
	}

	EXPECT_TRUE(jval["null"].isNull());
	EXPECT_EQ(true, jval["bool"].asBool());
	EXPECT_EQ(1.1, jval["number"].asNumber<double>());
	EXPECT_EQ("asd", jval["string"].asString());
	EXPECT_EQ(2, jval["array"][0].asNumber<int64_t>());
	EXPECT_EQ("qwerty", jval["array"][1].asString());

}

TEST(TestParse, invalid_json) {
	std::string input("{\"number\"\":1, \"str\":\"asd\"}");

	// Create a new parser, use default schema
	pbnjson::JDomParser parser(NULL);

	// Start stream parsing
	if (!parser.begin(JSchema::AllSchema())) {
		std::string error = parser.getError();
		return;
	}

	// parse input data part by part. Parts can be of any size, in this example it will be one byte.
	// Actually all data, that is available for the moment of call Parse, should be passed. It
	// will increase performance.
	for (std::string::const_iterator i = input.begin(); i != input.end() ; ++i) {
		if (!parser.feed(&(*i), 1)) {
			std::string error = parser.getError();
			return;
		}
	}

	if (!parser.end()) {
		std::string error = parser.getError();
		return;
	}

	// Get root JValue
	pbnjson::JValue json = parser.getDom();
}
