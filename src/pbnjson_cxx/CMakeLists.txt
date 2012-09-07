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

string(TOUPPER ${CXX_ENGINE} CXX_ENGINE_PKG)
set(${CXX_ENGINE_PKG}_STATIC ${STATIC_CXX_ENGINE})
find_package(${CXX_ENGINE_PKG} REQUIRED)
set(CXX_ENGINE_INCDIR ${${CXX_ENGINE_PKG}_INCLUDE_DIR})
set(CXX_ENGINE_LIBNAME ${${CXX_ENGINE_PKG}_LIBRARY})
message(STATUS "pbnjson_cxx: ${CXX_ENGINE} include directory ${CXX_ENGINE_INCDIR}, library ${CXX_ENGINE_LIBNAME}")

file(GLOB SHARED_SOURCE "./*.cpp" "../pbnjson_c/debugging.c")

if(WITH_VERBOSE_DEBUG)
	add_definitions(-DPJSON_LOG_DBG=1)
endif ()
if(WITH_VERBOSE_TRACE)
	add_definitions(-DPJSON_LOG_TRACE=1)
	add_definitions(-DPJSON_SCHEMA_TRACE=3)
endif()
add_definitions(-DPJSONCXX_SHARED)
add_definitions(-DPJSON_SHARED)
add_definitions(-DPJSONCXX_EXPORT)
add_definitions(-DLIBRARY_NAME=pbnjson_cpp)
    
add_library(pbnjson_cpp SHARED ${SHARED_SOURCE})
target_link_libraries(pbnjson_cpp pbnjson_c ${CXX_ENGINE_LIBNAME})

include_directories(${API_HEADERS}
                    ${API_HEADERS}/pbnjson
                    ${API_HEADERS}/pbnjson/c
                    ${API_HEADERS}/pbnjson/cxx
                    ${CXX_ENGINE_INCDIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../pbnjson_c
                    ${CMAKE_CURRENT_SOURCE_DIR})

if(CXX_ENGINE_LIBDIR)
	link_directories(${CXX_ENGINE_LIBDIR})
endif()

# Headers are installed by pbnjson_c
webos_build_library(NAME pbnjson_cpp NOHEADERS)

