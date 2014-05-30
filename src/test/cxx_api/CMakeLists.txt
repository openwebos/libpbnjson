# @@@LICENSE
#
#      Copyright (c) 2012-2014 LG Electronics, Inc.
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

set(TEST_LIBRARIES pbnjson_cpp)

######################### THE UNIT TESTS ############################


SET(CPPUnitTest
	TestDOM2
	TestJArray
	TestJValue2
	TestJValueGTest
	TestRegression
	TestJGenerator
	TestSchemaKeywords
	TestErrorHandler
	TestJValidatorErrorReporting
	TestSchema
	TestSchemaSanity2
	TestSchemaContact2
	TestParseCXX
	)

FOREACH(TEST ${CPPUnitTest})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} gtest gtest_main pthread)
	add_test(CPP.${TEST} ${TEST})
ENDFOREACH()

file(COPY "schemas" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DDATA_DIR="${CMAKE_CURRENT_BINARY_DIR}/schemas/")

add_definitions(-DSCHEMA_DIR="${CMAKE_CURRENT_BINARY_DIR}/../schemas/")

add_executable(LoadSchema LoadSchema.cpp)
target_link_libraries(LoadSchema ${TEST_LIBRARIES})
