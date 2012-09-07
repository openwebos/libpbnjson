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

# Project-wide information for the language bindings to find common directories
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src/CMakeModules/ ${CMAKE_MODULE_PATH})

macro(pick_engine_name variable_name static_value shared_value static_engine)
	if(${static_engine})
		set(${variable_name} ${static_value})
		message("${variable_name} is linking to static engine ${${variable_name}}")
	else()
		set(${variable_name} ${shared_value})
		message("${variable_name} is linking to shared engine ${${variable_name}}")
	endif()
endmacro()

set(C_ENGINE yajl)
set(STATIC_C_ENGINE FALSE)
set(CXX_ENGINE yajl)
set(STATIC_CXX_ENGINE FALSE)

pick_engine_name(C_ENGINE_LIBNAME STATIC_C_ENGINE ${C_ENGINE}_s ${C_ENGINE} STATIC_C_ENGINE)
pick_engine_name(CXX_ENGINE_LIBNAME ${CXX_ENGINE}_s ${CXX_ENGINE} STATIC_CXX_ENGINE)

# Configure the compilers for the libraries

message(STATUS "C engine's include directory is ${C_ENGINE_INCDIR}")
message(STATUS "C++ engine's include directory is ${CXX_ENGINE_INCDIR}")

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

# Build the JSON engine for the C library

# If the C++ JSON engine being used is different from the C one, then build it too
# CMake doesn't seem to be able to handle the same directory being added twice
if(DEFINED CXX_ENGINE AND NOT ${C_ENGINE} STREQUAL ${CXX_ENGINE})
	add_subdirectory(pjson_engine/${CXX_ENGINE})
endif()

set(WITH_SCHEMA TRUE CACHE BOOL "Build with schema support.  If built without, no input will fail schema validation.")
if(WITH_SCHEMA)
	set(WITH_PCRE TRUE CACHE BOOL "Build with PCRE support in schemas")
else()
	set(WITH_PCRE FALSE CACHE BOOL "Build with PCRE support in schemas" FORCE)
endif()

if(WITH_PCRE AND NOT WITH_SCHEMA)
	message(FATAL_ERROR "Conflicting option: WITH_PCRE set but WITH_SCHEMA ins't")
endif()

# XXX The tests don't compile
set(WITH_TESTS FALSE CACHE BOOL "Control building of unit tests")

set(WITH_VERBOSE_DEBUG FALSE CACHE BOOL "Enable verbose debug logging")
set(WITH_VERBOSE_TRACE FALSE CACHE BOOL "Enable tracing debug logging")

# build the language bindings
add_subdirectory(pbnjson_c)
add_subdirectory(pbnjson_cxx)

if(WITH_TESTS)
	set(WITH_QTCREATOR FALSE CACHE BOOL "Enable better Qt Creator integration")
	enable_testing()
	add_subdirectory(test)
else()
	message(STATUS "Skipping all unit tests")
endif()
