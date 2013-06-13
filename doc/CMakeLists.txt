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
# limitations under the License looks.
#
# LICENSE@@@

set(DOC_DIRNAME pbnjson-${WEBOS_API_VERSION})
set(PJ_DOC_OUTPUT ${WEBOS_BINARY_DOCUMENTATION_DIR}/${CMAKE_PROJECT_NAME}/html)
set(PJ_DOC_C_API_OUT ${PJ_DOC_OUTPUT}/c_api)
set(PJ_DOC_CXX_API_OUT ${PJ_DOC_OUTPUT}/cxx_api)
set(PJ_DOC_C_SRC_OUT ${PJ_DOC_OUTPUT}/c_internal)
set(PJ_DOC_CXX_SRC_OUT ${PJ_DOC_OUTPUT}/cxx_internal)

message(STATUS "** documentation output to: ${PJ_DOC_OUTPUT}")

foreach(DOC_OUTPUT_DIR ${PJ_DOC_C_API_OUT} ${PJ_DOC_CXX_API_OUT} ${PJ_DOC_C_SRC_OUT} ${PJ_DOC_CXX_SRC_OUT})
	file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR})
endforeach()

macro(doc_target custom_target lang doxy_filename)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${doxy_filename} ${CMAKE_CURRENT_BINARY_DIR}/${doxy_filename} @ONLY)
	add_custom_target(pbnjson-doc-${custom_target}_${lang}
	                  doxygen ${doxy_filename}
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	                  COMMENT "Generating ${custom_target} documentation for ${lang}")
endmacro()

add_custom_target(doc-api)
add_custom_target(doc-src)

doc_target(src c PBNJSON_InternalC.dxy)
doc_target(src cxx PBNJSON_InternalCXX.dxy)
doc_target(api c PBNJSON_PublicC.dxy)
doc_target(api cxx PBNJSON_PublicCXX.dxy)

add_dependencies(doc-api pbnjson-doc-api_c pbnjson-doc-api_cxx)
add_dependencies(doc-src pbnjson-doc-src_c pbnjson-doc-src_cxx)
add_dependencies(pbnjson-doc-src_c pbnjson-doc-api_c)
add_dependencies(pbnjson-doc-src_cxx pbnjson-doc-api_cxx)

add_custom_target(docs ALL)

add_dependencies(docs doc-api doc-src)
