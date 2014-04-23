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

add_definitions(-D_GNU_SOURCE)
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
set(WITH_DEBUG_POINTERS TRUE CACHE BOOL "In debug builds, perform extra checks on pointer manipulations")
if(CMAKE_BUILD_TYPE_LOWER STREQUAL "release")
	set(WITH_DEBUG_POINTERS FALSE CACHE BOOL "" FORCE)
endif()
if(WITH_DEBUG_POINTERS)
	webos_add_compiler_flags(DEBUG -DDEBUG_FREED_POINTERS=1)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# fmessage-length=0: Make sure that errors & messages are printed on 1 line
	webos_add_compiler_flags(ALL -fmessage-length=0 -Wall)
	webos_add_compiler_flags(DEBUG -O0 -DDEBUG -D_DEBUG -DPJSON_LOG_INFO -DPJSON_LOG_STDOUT)
	webos_add_compiler_flags(RELEASE -DNDEBUG -g)
else()
	# don't take out this message - this is the default fall-through case when the
	# compiler cannot be determined.  instead, add compiler detection as necessary
	message("WARNING - not using GCC for compilation - please provide flags to enable C99")
	webos_add_compiler_flags(ALL -D_BSD_SOURCE)
endif()

webos_add_linker_options(ALL --no-undefined)

set(WITH_TESTS FALSE CACHE BOOL "Control building of unit tests")

set(WITH_VERBOSE_DEBUG FALSE CACHE BOOL "Enable verbose debug logging")
set(WITH_VERBOSE_TRACE FALSE CACHE BOOL "Enable tracing debug logging")

# build the language bindings
add_subdirectory(pbnjson_c)
add_subdirectory(pbnjson_cxx)
add_subdirectory(pbnjson_validate)

if(WITH_TESTS)
	set(WITH_QTCREATOR FALSE CACHE BOOL "Enable better Qt Creator integration")
	enable_testing()
	add_subdirectory(test)
else()
	message(STATUS "Skipping all unit tests")
endif()
