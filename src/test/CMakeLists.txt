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

include(CheckIncludeFileCXX)

# for some reason there's some kind of conflict (HAVE_CXXABI_H refuses to get set)
check_include_file_cxx(cxxabi.h HAVE_CXXABI_H_)
check_include_files(execinfo.h HAVE_EXECINFO_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/execinfo.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pjson_execinfo.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(APPLE)
	# Problems w/ 32-bit vs 64-bit on snow leopard w/ Qt
	# Force 32-bit
	set(CMAKE_OSX_ARCHITECTURES)
	add_definitions(-m32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
endif()

# Google Test setup
webos_use_gtest()

add_subdirectory(c_api)
add_subdirectory(cxx_api)

if(WITH_QTCREATOR)
	file(GLOB_RECURSE SCHEMAS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.schema *.json)
	foreach(schema ${SCHEMAS})
		configure_file(${schema} ${schema} COPYONLY)
	endforeach()
endif()

file(COPY "schemas" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DSCHEMA_DIR="${CMAKE_CURRENT_BINARY_DIR}/schemas/")
