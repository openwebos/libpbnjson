# @@@LICENSE
#
# Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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

cmake_minimum_required(VERSION 2.8.7)
project(pbnjson)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC3)
webos_component(1 3 0)

include(FindPkgConfig)

set(API_HEADERS ${CMAKE_SOURCE_DIR}/include/public)
set(WITH_GTEST_SRC "/usr/src/gtest" CACHE PATH "Path to Google Test source code")

add_subdirectory(src)

if(WITH_DOCS)
	add_subdirectory(doc)
else()
	message(STATUS "Skipping document generation")
endif()

if(WITH_TESTS)
	enable_testing()
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
else()
	message(STATUS "Skipping automatic tests")
endif()
