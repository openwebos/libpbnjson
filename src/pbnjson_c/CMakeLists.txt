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


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sys_malloc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/sys_malloc.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pjson_syslog.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pjson_syslog.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/regexp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/regexp.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/strnlen.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/strnlen.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/isatty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/isatty.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC")

add_subdirectory(validation)

add_library(
	jvalue
	STATIC
	debugging.c
	jobject.c
	jvalue/num_conversion.c
	)
set_target_properties(jvalue PROPERTIES DEFINE_SYMBOL PJSON_SHARED)

add_library(
	pbnjson_c
	SHARED
	jgen_stream.c
	jvalue_tostring.c
	jparse_stream.c
	jschema.c
	jvalidation.c
	)
target_link_libraries(pbnjson_c jvalue schema_validation ${GLIB2_LDFLAGS} ${YAJL_LDFLAGS})
set_target_properties(pbnjson_c PROPERTIES DEFINE_SYMBOL PJSON_SHARED)

include_directories(
	${API_HEADERS}
	${API_HEADERS}/pbnjson
	${API_HEADERS}/pbnjson/c
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

if(WITH_VERBOSE_DEBUG)
	add_definitions(-DPJSON_LOG_DBG=1)
endif()
if(WITH_VERBOSE_TRACE)
	add_definitions(-DPJSON_LOG_TRACE=1)
	add_definitions(-DPJSON_SCHEMA_TRACE=3)
endif()
add_definitions(-DLIBRARY_NAME=pbnjson_c)
add_definitions(-DPJSON_EXPORT)

webos_build_library(NAME pbnjson_c)
webos_build_pkgconfig(files/pkgconfig/pbnjson_c)

if(WITH_TESTS)
	enable_testing()
endif()
