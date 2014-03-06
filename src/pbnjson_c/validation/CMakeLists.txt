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


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_custom_target(
	json_schema_grammar echo "Creating json_schema_grammar.c"
	)

add_custom_command(
	SOURCE json_schema_grammar.y
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/apply_lemon.sh
		${LEMON}
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		json_schema_grammar.y
	TARGET json_schema_grammar
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/json_schema_grammar.c
	        ${CMAKE_CURRENT_BINARY_DIR}/json_schema_grammar.h
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/json_schema_grammar.y
	        ${CMAKE_CURRENT_SOURCE_DIR}/apply_lemon.sh
	)

set(grammar_c ${CMAKE_CURRENT_BINARY_DIR}/json_schema_grammar.c)
set_source_files_properties(${grammar_c} GENERATED)
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${API_HEADERS} ${API_HEADERS}/pbnjson ${API_HEADERS}/pbnjson/c
	)

add_custom_target(
	schema_keywords echo "Creating jscon_schema_keywords.h"
	)

function(gperf_generate source target)
	add_custom_target(${target} echo "Creating ${target}")
	add_custom_command(
		SOURCE ${source}
		TARGET ${target}
		COMMAND ${GPERF} -t ${CMAKE_CURRENT_SOURCE_DIR}/${source} > ${target}
		OUTPUTS ${target}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
		)
endfunction()

gperf_generate(schema_keywords.gperf schema_keywords.h)
gperf_generate(instance_types.gperf instance_types.h)

add_library(
	schema_validation
	STATIC
	${CMAKE_CURRENT_BINARY_DIR}/schema_keywords.h
	${CMAKE_CURRENT_BINARY_DIR}/instance_types.h
	${grammar_c}
	additional_feature.c
	array_items.c
	array_validator.c
	boolean_feature.c
	boolean_validator.c
	count_feature.c
	definitions.c
	error_code.c
	everything_validator.c
	feature.c
	generic_validator.c
	jvalue_feature.c
	nothing_validator.c
	null_validator.c
	number.c
	number_feature.c
	number_validator.c
	string_validator.c
	object_properties.c
	object_required.c
	object_validator.c
	combined_types_validator.c
	combined_validator.c
	parser_api.c
	parser_context.c
	pattern.c
	reference.c
	schema_parsing.c
	type_parser.c
	uri_scope.c
	uri_resolver.c
	validation_api.c
	validation_event.c
	validation_state.c
	validator.c
	)

target_link_libraries(
	schema_validation
	LINK_PRIVATE
	jvalue
	${GLIB2_LDFLAGS} ${YAJL_LDFLAGS} ${URIPARSER_LDFLAGS} ${GMP_LIBRARY}
	)

if(WITH_TESTS)
	add_subdirectory(test)
	enable_testing()
endif()
