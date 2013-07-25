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

if(NOT QT_FOUND)
	message(WARNING "Qt not found - skipping building many tests for C api")
endif()

include_directories(${API_HEADERS})
webos_add_compiler_flags(ALL -Wall "-std=gnu++0x")

# Find CJSON for comparison benchmarks
pkg_check_modules(CJSON cjson)
if(NOT CJSON_FOUND)
	set(CJSON_INCLUDE_DIR "")
	set(CJSON_LIBRARIES "")
	set(HAVE_CJSON false)
endif()

find_package(YAJL)
if(NOT YAJL_FOUND)
	set(YAJL_INCLUDE_DIRS "")
	set(YAJL_LIBRARIES "")
	set(FOUND_YAJL false)
endif()

######################### TEST CONFIGURATION ########################
set(TEST_LIBRARIES pbnjson_c ${CJSON_LIBRARIES} ${YAJL_LIBRARIES})

include(CheckSymbolExists)
if(CJSON_FOUND)
	check_symbol_exists("json_object_new_object" "json_object.h" HAVE_CJSON)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cjson.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/cjson.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yajl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/yajl.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${YAJL_INCLUDE_DIRS} ${CJSON_INCLUDE_DIRS})

list(APPEND TEST_PARSE_FILES file_parse_test)

foreach(i ${TEST_PARSE_FILES})
	configure_file(${i}.json ${i}.json COPYONLY)
	configure_file(${i}.schema ${i}.schema COPYONLY)
endforeach()



######################### THE UNIT TESTS ############################
qt_hdrs(test_sax
	TestSAX.h
)

src(test_sax
	TestSAX.cpp
	JSXMLConverter.cpp
)

qt_hdrs(test_schema
	TestSchema.h
)

src(test_schema
	TestSchema.cpp
)
qt_hdrs(test_schema2
	TestSchemaSanity.h
)

src(test_schema2
	TestSchemaSanity.cpp
)

src(smoketest_mem_badinput
	SmokeTestMemLeakBadInput.cpp
)

include_directories(${C_ENGINE_INCDIR})

set(test_parse_test_list
	testParseDoubleAccuracy
	testParseFile
)

set(test_sax_test_list
	testGenerator
	testParser
)

set(test_schema2_test_list
	testSimpleSchema
	testSchemaReuse
)

add_qt_test(test_sax "SAX API")
add_qt_test(test_schema2 "Schema sanity checks")
add_regular_test(smoketest_mem_badinput "C memory leak smoke test for parsing invalid input")

add_schema_test(test_schema "Number sanity check" ${CMAKE_CURRENT_SOURCE_DIR}/../schemas/sanity/ SimpleNumber)
add_schema_test(test_schema "Contact" ${CMAKE_CURRENT_SOURCE_DIR}/../schemas/contact/ Contact)

SET(UnitTest
	TestParse
	TestDOM
	TestJobject
	TestYAJL
	)

FOREACH(TEST ${UnitTest})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} gtest gtest_main pthread)
	add_test(C.${TEST} ${TEST})
ENDFOREACH()

add_executable(TestPerformance TestPerformance.cpp)
target_link_libraries(TestPerformance ${TEST_LIBRARIES} gtest gtest_main pthread)
