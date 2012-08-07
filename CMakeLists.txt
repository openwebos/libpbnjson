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

if (NOT DEFINED PBNJSON_MAJOR)
	set(PBNJSON_MAJOR 1)
endif (NOT DEFINED PBNJSON_MAJOR)

if (NOT DEFINED PBNJSON_MINOR)
	set(PBNJSON_MINOR 1)
endif (NOT DEFINED PBNJSON_MINOR)

if (NOT DEFINED PBNJSON_MICRO)
	set(PBNJSON_MICRO 0)
endif (NOT DEFINED PBNJSON_MICRO)

set(PBNJSON_VERSION ${PBNJSON_MAJOR}.${PBNJSON_MINOR}.${PBNJSON_MICRO})

set(API_HEADERS ${CMAKE_SOURCE_DIR}/include/public)

add_subdirectory(src)

if (WITH_DOCS)
	add_subdirectory(doc)
else()
	message(STATUS "Skipping document generation")
endif ()
