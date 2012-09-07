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

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCXXSourceCompiles)
include(CheckFunctionExists)
set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_include_files("malloc.h" HAVE_MALLOC_H)
check_include_files("syslog.h" HAVE_SYSLOG_H)
check_include_files("regex.h" HAVE_POSIX_REGEXP)
check_symbol_exists("vsyslog" "stdarg.h;syslog.h" HAVE_VSYSLOG)
check_symbol_exists("vfprintf" "stdarg.h;stdio.h" HAVE_VFPRINTF)
check_symbol_exists("__assert_fail" "assert.h" HAVE_ASSERT_FAIL)
check_function_exists(strnlen HAVE_STRNLEN)
check_function_exists(isatty HAVE_ISATTY)

# Check for GCC atomics	
check_cxx_source_compiles(	
	"int main() {
		int f;
		__sync_fetch_and_add(&f, 1);
		__sync_fetch_and_sub(&f, 1);
		__sync_add_and_fetch(&f, 1);
		__sync_sub_and_fetch(&f, 1);
		__sync_bool_compare_and_swap(&f, 1, 0);
		return 0;
	}"
	HAVE_GCC_ATOMICS)

if(WITH_PCRE)
	pkg_check_modules(PCRE libpcre)
	if(PCRE_FOUND)
		set(HAVE_PCRE 1)
	endif()
endif()

string(TOUPPER ${C_ENGINE} C_ENGINE_PKG)
set(${C_ENGINE_PKG}_STATIC ${STATIC_C_ENGINE})
find_package(${C_ENGINE_PKG} REQUIRED)
set(C_ENGINE_INCDIR ${${C_ENGINE_PKG}_INCLUDE_DIRS})
set(C_ENGINE_LIBNAME ${${C_ENGINE_PKG}_LIBRARIES})
message(STATUS "pbnjson_c: ${C_ENGINE} include directory ${C_ENGINE_INCDIR}, library ${C_ENGINE_LIBNAME}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sys_malloc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/sys_malloc.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pjson_syslog.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pjson_syslog.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/regexp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/regexp.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/strnlen.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/strnlen.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/isatty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/isatty.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assert_compat.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/assert_compat.h)

webos_add_compiler_flags(ALL -std=c99)

file(GLOB SHARED_SOURCE "./*.c" "./jvalue/*.c")

set(STATIC_SOURCE ${SHARED_SOURCE})
    
add_library(pbnjson_c SHARED ${SHARED_SOURCE})
target_link_libraries(pbnjson_c ${C_ENGINE_LIBNAME} ${PCRE_LDFLAGS})
set_target_properties(pbnjson_c PROPERTIES DEFINE_SYMBOL PJSON_SHARED)

if(WITH_STATIC)
	add_library(pbnjson_c_s STATIC ${STATIC_SOURCE})
	target_link_libraries(pbnjson_c_s ${C_ENGINE_LIBNAME} ${PCRE_LDFLAGS})
endif()

include_directories(${API_HEADERS} ${API_HEADERS}/pbnjson ${API_HEADERS}/pbnjson/c ${C_ENGINE_INCDIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${PCRE_INCLUDE_DIRS})
if(C_ENGINE_LIBDIR)
	link_directories(${C_ENGINE_LIBDIR})
endif()

if(WITH_VERBOSE_DEBUG)
	add_definitions(-DPJSON_LOG_DBG=1)
endif()
if(WITH_VERBOSE_TRACE)
	add_definitions(-DPJSON_LOG_TRACE=1)
	add_definitions(-DPJSON_SCHEMA_TRACE=3)
endif()
add_definitions(-DLIBRARY_NAME=pbnjson_c)
add_definitions(-DPJSON_EXPORT)

if(NOT WITH_SCHEMA)
	message(STATUS "Compiling without schema support: WITH_SCHEMA=${WITH_SCHEMA}")
	add_definitions(-DBYPASS_SCHEMA=1)
else()
	message(STATUS "Compiling with schema support: WITH_SCHEMA=${WITH_SCHEMA}")
	add_definitions(-DBYPASS_SCHEMA=0)
endif()

webos_build_library(NAME pbnjson_c)

