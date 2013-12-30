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

#include "../parser_api.h"
#include "../uri_resolver.h"
#include "../validation_api.h"
#include <gtest/gtest.h>
#include <iostream>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace std;
namespace bi = boost::interprocess;

namespace {

typedef pair<char const *, size_t> RegionT;

void OnError(size_t offset, SchemaErrorCode error, char const *message, void *ctxt)
{
	RegionT const *region = reinterpret_cast<RegionT const *>(ctxt);
	char const *error_point = region->first + offset;
	char const *begin = error_point;
	for (unsigned i = 0; i != 24 && i < offset && *begin != '\n'; ++i)
		--begin;
	char const *end = error_point;
	for (unsigned i = 0; i != 24 && i < region->second - offset && *end != '\n'; ++i)
		++end;
	copy(begin, end, ostream_iterator<char>(cout));
	cout << endl;
	for (int i = 0, n = error_point - begin; i != n; ++i)
		cout << " ";
	cout << "^" << endl;
	cerr << "Error at " << offset << ": " << message << endl;
}

} //namespace

TEST(TestMetaSchema, First)
{
	bi::file_mapping file(SCHEMAS_DIR "schema", bi::read_only);
	bi::mapped_region region(file, bi::read_only);

	auto uri_resolver = uri_resolver_new();
	ASSERT_TRUE(uri_resolver != NULL);

	RegionT ctxt(reinterpret_cast<char const *>(region.get_address()),
	             region.get_size());
	auto v = parse_schema_n((char const *) region.get_address(), region.get_size(),
	                        uri_resolver, "file://" SCHEMAS_DIR "schema",
	                        &OnError, &ctxt);
	ASSERT_TRUE(v != NULL);

	auto data = reinterpret_cast<char const *>(region.get_address());
	auto data_size = region.get_size();

	ValidationError error = { 0 };
	EXPECT_TRUE(validate_json_n(data, data_size, v, uri_resolver, &error));
	if (error.error != VEC_OK)
	{
		size_t line = 1 + std::count(data, data + error.offset, '\n');
		cerr << "Error " << error.error << " at line " << line << ": ";
		cerr << ValidationGetErrorMessage(error.error) << endl;
	}

	validator_unref(v);
	uri_resolver_free(uri_resolver);
}
