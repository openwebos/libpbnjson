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

include_directories(${API_HEADERS})
webos_add_compiler_flags(ALL -Wall "-std=gnu++0x")

# Find CJSON for comparison benchmarks
pkg_check_modules(CJSON cjson)
if(NOT CJSON_FOUND)
	set(CJSON_INCLUDE_DIR "")
	set(CJSON_LIBRARIES "")
	set(HAVE_CJSON false)
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

SET(UnitTest
	SmokeTestMemLeakBadInput
	TestParse
	TestDOM
	TestJvalue
	TestJobject
	TestSchemaSanity
	TestSchemaContact
	TestSchemaUniqueItems
	)

FOREACH(TEST ${UnitTest})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} gtest gtest_main pthread)
	add_test(C.${TEST} ${TEST})
ENDFOREACH()

######################### THE PERFORMANCE TESTS ############################

SET(PerformanceTests
	TestPerformance
	TestSchemaPerformance
	TestJobjectPerformance
	)

FOREACH(TEST ${PerformanceTests})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} gtest gtest_main pthread)
ENDFOREACH()

add_definitions(-DSCHEMA_DIR="${CMAKE_CURRENT_BINARY_DIR}/../schemas/")
