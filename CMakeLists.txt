# @@@LICENSE
#
# Copyright (c) 2009-2013 LG Electronics, Inc.
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
webos_component(2 0 0)

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL -Wall ${GLIB2_CFLAGS_OTHER})

set(API_HEADERS ${CMAKE_SOURCE_DIR}/include/public)
set(WITH_GTEST_SRC "/usr/src/gtest" CACHE PATH "Path to Google Test source code")

add_subdirectory(src)

if(WITH_DOCS)
	add_subdirectory(doc)
else()
	message(STATUS "Skipping document generation")
endif()

if(WITH_TESTS)
	include(CTest)
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
else()
	message(STATUS "Skipping automatic tests")
endif()
