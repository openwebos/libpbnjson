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
webos_component(2 1 0)

include(FindPkgConfig)

find_library(GMP_LIBRARY gmp ${LIB_INSTALL_DIR})

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL -Wall -D__STRICT_ANSI__ ${GLIB2_CFLAGS_OTHER})

# YAJL-1 doesn't ship any pkg-check module
pkg_check_modules(YAJL yajl)
if("${YAJL_VERSION}" STREQUAL "")
	message(STATUS "Looking for YAJL in library paths")
	find_library(YAJL_LDFLAGS yajl ${LIB_INSTALL_DIR})
	if(${YAJL_LDFLAGS} STREQUAL "YAJL_LDFLAGS-NOTFOUND")
		message(FATAL_ERROR "Cannot find YAJL")
	else()
		message(STATUS "Found ${YAJL_LDFLAGS}")
	endif()
else()
	include_directories(${YAJL_INCLUDE_DIRS})
endif()

pkg_check_modules(URIPARSER REQUIRED liburiparser)
include_directories(${URIPARSER_INCLUDE_DIRS})

find_program(GPERF NAMES gperf DOC "GNU gperf perfect hash function generator")
if(${GPERF} STREQUAL "GPERF-NOTFOUND")
	message(FATAL_ERROR "Cannot find GNU gperf executable")
endif()

find_program(LEMON NAMES lemon DOC "The LEMON LALR(1) parser generator")
if(${LEMON} STREQUAL "LEMON-NOTFOUND")
	message(FATAL_ERROR "Cannot find lemon LALR(1) parser generator")
endif()

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
