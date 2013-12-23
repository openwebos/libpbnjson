# @@@LICENSE
#
#      Copyright (c) 2009-2013 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@


enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
include_directories(${WITH_GTEST_SRC}/include)

SET(UnitTests
	TestValidator
	TestValidatorEquals
	TestGenericValidator
	TestNullValidator
	TestBooleanValidator
	TestNumber
	TestNumberValidator
	TestIntegerValidator
	TestStringValidator
	TestArrayValidator
	TestObjectValidator
	TestCombinedTypesValidator
	TestAllOfValidator
	TestAnyOfValidator
	TestOneOfValidator
	TestParser
	TestJson
	TestUriScope
	TestUriResolver
	TestDefinitions
	TestReference
	TestArrayValidator2
	TestEnum
	TestMetaSchema
	)

FOREACH(TEST ${UnitTests})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} schema_validation gtest gtest_main ${GLIB2_LDFLAGS})
	add_test(Validation.${TEST} ${TEST})
ENDFOREACH()

file(GLOB_RECURSE SCHEMAS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} schemas/*)
foreach(schema ${SCHEMAS})
	configure_file(${schema} ${schema} COPYONLY)
endforeach()

add_definitions(-DSCHEMAS_DIR="${CMAKE_CURRENT_BINARY_DIR}/schemas/")
