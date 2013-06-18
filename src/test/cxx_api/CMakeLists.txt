# @@@LICENSE
#
#      Copyright ©2009-2013 Zenith Electronics LLC, a subsidiary of LG Electronics USA, Inc. All Rights Reserved.
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
	message(STATUS "Qt not found - skipping building many tests for C++ api")
endif()

src(smoketest_mem
	SmokeTestMemLeak.cpp
)

include_directories(${API_HEADERS})
webos_add_compiler_flags(ALL -Wall "-std=gnu++0x")

set(TEST_LIBRARIES pbnjson_cpp)

######################### THE UNIT TESTS ############################

qt_hdrs(test_cppsax
	TestSAX.h
)

src(test_cppsax
	TestSAX.cpp
)

src(test_yajl
	TestYAJL.cpp
)

qt_hdrs(test_cppschema
	TestSchema.h
)

src(test_cppschema
	TestSchema.cpp
)

qt_hdrs(test_regressions
	TestRegression.h
)

src(test_regressions
	TestRegression.cpp
)

qt_hdrs(test_jvalue
	TestJValue.h
)

src(test_jvalue
	TestJValue.cpp
)

set(test_regressions_test_list
	testNOV99444
)

include_directories(${C_ENGINE_INCDIR})

add_regular_test(smoketest_mem "C++ Smoke Test of Memory Usage")
#add_qt_test(test_cppsax "pjson C++ binding SAX test")
#add_qt_test(test_num_conversion "Numeric conversion Unit Tests")
add_schema_test(test_cppschema "C++ Number sanity check" ${CMAKE_CURRENT_SOURCE_DIR}/../schemas/sanity/ SimpleNumber)
add_schema_test(test_cppschema "C++ Contact" ${CMAKE_CURRENT_SOURCE_DIR}/../schemas/contact/ Contact)
add_qt_test(test_regressions "Regression testing")
add_qt_test(test_jvalue "Test JValue class")


add_executable(test_cppdom TestDOM.cpp)
target_link_libraries(test_cppdom ${TEST_LIBRARIES} gtest gtest_main pthread)
add_test(CPP.TestDOM test_cppdom)

add_executable(test_jarray TestJArray.cpp)
target_link_libraries(test_jarray ${TEST_LIBRARIES} gtest gtest_main pthread)
add_test(CPP.TestJArray test_jarray)
