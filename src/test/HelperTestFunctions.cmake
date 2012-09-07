# @@@LICENSE
#
#      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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

find_package(Valgrind)

# Variable compatibility with existing build scripts
# Deprecated in favour of setting WITH_VALGRIND to 'memcheck'
if(DEFINED MEMCHK)
	set(WITH_VALGRIND "memcheck" CACHE STRING "When running unit tests, the tool to use when running valgrind (memcheck, callgrind, etc)")
else()
	set(WITH_VALGRIND "" CACHE STRING "When running unit tests, the tool to use when running valgrind (memcheck, callgrind, etc)")
endif()

set(VALGRIND_TOOL ${WITH_VALGRIND})

set(WITH_VALGRIND_memcheck_OPTIONS --leak-check=full --error-exitcode=127 CACHE STRING "The options to pass when running memcheck")
set(WITH_VALGRIND_callgrind_OPTIONS "" CACHE STRING "The options to pass when running callgrind")

if(VALGRIND_FOUND AND VALGRIND_TOOL)
	# G_SLICE=always-malloc is needed for testing w/ valgrind
	# properly (otherwise tests may provide false positives due to
	# Qt using Glib & the slice allocator never freeing memory)
	# Setting test environment variables from CMake are only supported
	# as of 2.8
	if(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
		macro(conf_valgrind_env test_name)
			set(SUPRESS_GSLICE_WARNING $ENV{SUPRESS_GSLICE_WARNING})
			if (NOT SUPRESS_GSLICE_WARNING)
				message(WARNING "CMake version is too old - you need to manually set G_SLICE=always-malloc when running the tests under valgrind (set SUPRESS_GSLICE_WARNING env variable to suppress this warning)")
			endif()
		endmacro()
	else()
		macro(conf_valgrind_env test_name)
			set_property(TEST '${${test_name}}' APPEND PROPERTY ENVIRONMENT
				"G_SLICE=always-malloc"
			)
		endmacro()
	endif()
endif()

# Locate Qt for testing
# Qt4 preamble
find_package(Qt4 4.5 COMPONENTS QtCore QtTestLib)

# You'd think that if I asked a minimum Qt version, it wouldn't
# set QT_FOUND if that minimum version wasn't found.
# This is probably a bug in CMake or the FindQt4.cmake script.
# In the future, they might define this variable, so let's make this
# forward compatible
if(NOT QT_VERSION)
	set(QT_VERSION "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
	message(WARNING "QT_VERSION not found - manually constructed as ${QT_VERSION}")
endif()

if("4.5.0" VERSION_GREATER "${QT_VERSION}")exe_name
	message(STATUS "Found Qt version ${QT_VERSION} is too old for unit tests - pbnjson tests will not be built")
	set(QT_FOUND FALSE)
endif()

if(QT_FOUND)
	set(QT_USE_QTTEST TRUE)
	set(QT_DONT_USE_QTCORE FALSE)
	set(QT_DONT_USE_QTGUI TRUE)
	set(QT_USE_QTXML TRUE)

	include(${QT_USE_FILE})
	if(NOT QT_LIBRARIES)
	    if(APPLE)
		list(APPEND QT_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
		find_path(QT_INCLUDE_DIR QByteArray)
		message(STATUS "Seems there's a problem with the FindQt4.cmake in CMake for Apple - QT_LIBRARIES not set.  Manually setting it to ${QT_LIBRARIES}")
	    else()
		message(FATAL_ERROR "Qt libraries found, but QT_LIBRARIES variable not set by FindQt4.cmake - tests will not compile")
	    endif()
	elseif(NOT QT_QTTEST_FOUND)
		message(FATAL_ERROR "Qt4 found, but QtTest component wasn't - tests will not compile")
	elseif(NOT_QT_QTCORE_FOUND)
		message(FATAL_ERROR "Qt4 found, but QtCore component wasn't - tests will not compile")
	endif()

	######################### HELPER FUNCTIONS ############################    
	# exe_name - the name of the executable
	# ... - A list of headers to MOC
	# defines the variable ${exe_name}_qthdrs to the given list
	macro(qt_hdrs exe_name)
		set(${exe_name}_qthdrs ${ARGN})
	endmacro()
else()
	macro(qt_hdrs exe_name)
	endmacro()
endif()

# exe_name - the name of the executable
# ... - A list of source files to include in the executable
# defines the variable ${exe_name}_src to the given list
macro(src exe_name)
	set(${exe_name}_src ${ARGN})
endmacro()

# Variable compatibility with existing build scripts
# Deprecated in favour of setting WITH_PERFORMANCE_TESTS
if(DEFINED ADD_PERFORMANCE_TEST)
	set(WITH_PERFORMANCE_TESTS ${ADD_PERFORMANCE_TEST} CACHE BOOL "Include performance unit tests")
else )
	set(WITH_PERFORMANCE_TESTS FALSE CACHE BOOL "Include performance unit tests")
endif )

macro(conf_valgrind_prefix)
	if (NOT VALGRIND_FOUND AND VALGRIND_TOOL)
		message(FATAL_ERROR "Requesting running tests under valgrind, but valgrind not found")
	elseif(VALGRIND_TOOL)
		if(NOT VALGRIND_TOOL STREQUAL "callgrind" AND NOT VALGRIND_TOOL STREQUAL "memcheck" AND NOT VALGRIND_TOOL STREQUAL "cachegrind" AND NOT VALGRIND_TOOL STREQUAL "massif" AND NOT VALGRIND_TOOL STREQUAL "helgrind")
			message(WARNING "Valgrind tool ${VALGRIND_TOOL} not recognized")
		elseif(NOT DEFINED WITH_VALGRIND_${VALGRIND_TOOL}_OPTIONS)
			message(WARNING "Valgrind tool ${VALGRIND_TOOL} doesn't have any options specified")
		endif()

		set(VALGRIND_OPTIONS "${WITH_VALGRIND_${VALGRIND_TOOL}_OPTIONS}")
		set(cmd_prefix valgrind --tool=${VALGRIND_TOOL} ${VALGRIND_OPTIONS})
	else()
		unset(cmd_prefix)
	endif()
endmacro()

if(QT_FOUND)
	# exe_name - the name of the executable
	#            make sure you use qt_hdrs & src macros so that variable naming is consistent
	# test_name - the friendly name of the test
	# ... - An optional list of command line arguments to pass to the executable
	function(add_qt_test exe_name test_name)
		conf_valgrind_prefix()

		if(NOT TARGET ${exe_name})
			# Specify MOC
			qt4_wrap_cpp(${exe_name}_mocsrc ${${exe_name}_qthdrs})
			
			# Create executable
			add_executable(${exe_name} ${${exe_name}_src} ${${exe_name}_mocsrc})
			
			# Libraries to link against
			target_link_libraries(${exe_name}
				${QT_LIBRARIES}
				${TEST_LIBRARIES}
			)
		endif()

		if(NOT DEFINED ${exe_name}_test_list)
			if (VALGRIND_TOOL)
				set(test_name "Valgrind ${VALGRIND_TOOL}: ${test_name}")
			endif()
			string(REPLACE " " "\\ " test_name ${test_name}) 
			add_test('${test_name}' ${cmd_prefix} ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} ${ARGN})
			conf_valgrind_env(test_name)
		else()
			foreach(test_name ${${exe_name}_test_list})
				set(tmp_test_name "${test_name} : ${test_name}")
				if(VALGRIND_TOOL)
					set(tmp_test_name "Valgrind ${VALGRIND_TOOL}: ${tmp_test_name}")
				endif()
				string(REPLACE " " "\\ " tmp_test_name ${tmp_test_name}) 
				add_test('${tmp_test_name}' ${cmd_prefix} ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} ${test_name})
				conf_valgrind_env(tmp_test_name)
			endforeach()
		endif()
	endfunction()

	function(add_schema_test_ex exe_name test_name schema_path schema_name is_valid)
		if(is_valid)
			set(RELATIVE_PATH "${schema_path}/input_valid")
			# the empty prefix is so that test names line up nicely on output
			set(test_name "        ${test_name}")
			set(PASS_VALUE 1)
		else()
			set(RELATIVE_PATH "${schema_path}/input_invalid")
			SET(test_name "Invalid ${test_name}")
			set(PASS_VALUE 0)
		endif()
		file(GLOB test_files RELATIVE "${RELATIVE_PATH}" "${RELATIVE_PATH}/${schema_name}.*.json")
		if(test_files)
			foreach(test_file ${test_files})
				set(${exe_name}_input ${test_file}_${PASS_VALUE})
				add_qt_test("${exe_name}" "Schema API : ${test_name} w/ input file ${test_file}" -schema "${schema_path}/${schema_name}.schema" -input "${RELATIVE_PATH}/${test_file}"  -pass ${PASS_VALUE})
			endforeach()
		else()
			message(SEND_ERROR "${schema_name} has no test cases (is_valid = ${is_valid}): RELATIVE_PATH = '${RELATIVE_PATH}', test_files = '${test_files}'")
		endif()
	endfunction()

	function(add_schema_test exe_name test_name schema_path schema_name)
		add_schema_test_ex("${exe_name}" "${test_name}" "${schema_path}" "${schema_name}" TRUE)
		add_schema_test_ex("${exe_name}" "${test_name}" "${schema_path}" "${schema_name}" FALSE)
	endfunction()

else()
	macro(add_qt_test)
	endmacro()

	macro(add_schema_test_ex exe_name test_name schema_path schema_name is_valid)
	endmacro()

	macro(add_schema_test exe_name test_name schema_path schema_name)
	endmacro()

endif()

# exe_name - the name of the executable
#            make sure you use qt_hdrs & src macros so that variable naming is consistent
# test_name - the friendly name of the test
# ... - An optional list of command line arguments to pass to the executable
function(add_regular_test exe_name test_name)
    conf_valgrind_prefix()

    if(NOT TARGET ${exe_name})
	# Create executable
	add_executable(${exe_name} ${${exe_name}_src})
	
	# Libraries to link against
	target_link_libraries(${exe_name}
		${TEST_LIBRARIES}
	)
    endif()

    if(NOT DEFINED ${exe_name}_test_list)
	if(VALGRIND_TOOL)
		set(test_name "Valgrind: ${test_name}")
	endif()
	string(REPLACE " " "\\ " test_name ${test_name}) 
	add_test('${test_name}' ${cmd_prefix} ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} ${ARGN})
	conf_valgrind_env(test_name)
    else()
	foreach(test_name ${${exe_name}_test_list})
		set(tmp_test_name "${test_name} : ${test_name}")
		if (VALGRIND_TOOL)
			set(tmp_test_name "Valgrind: ${tmp_test_name}")
		endif()
		string(REPLACE " " "\\ " tmp_test_name ${tmp_test_name}) 
		add_test('${tmp_test_name}' ${cmd_prefix} ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} ${test_name})
		#set_property(TEST '${tmp_test_name}' APPEND PROPERTY ENVIRONMENT G_SLICE=always-malloc)
		conf_valgrind_env(tmp_test_name)
	endforeach()
    endif()
endfunction()

