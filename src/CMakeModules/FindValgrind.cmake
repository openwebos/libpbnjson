# @@@LICENSE
#
#      Copyright (c) 2012-2013 LG Electronics, Inc.
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

# This will define:
# VALGRIND_FOUND - system has valgrind
# VALGRIND_INCLUDE_DIRS - include directories necessary to compile for valgrind
# VALGRIND_PROGRAM - the valgrind executable
#

include(FindPackageHandleStandardArgs)

# Find the include directories
find_path(VALGRIND_INCLUDE_DIRS NAMES memcheck.h PATHS /usr/include /usr/include/valgrind /usr/local/include /usr/local/include/valgrind)

# Find the program
find_program(VALGRIND_PROGRAM NAMES valgrind PATHS /usr/bin /usr/local/bin)

find_package_handle_standard_args(VALGRIND DEFAULT_MSG VALGRIND_INCLUDE_DIRS VALGRIND_PROGRAM)

if(VALGRIND_FOUND)
	mark_as_advanced(VALGRIND_INCLUDE_DIRS VALGRIND_PROGRAM)
else()
	unset(VALGRIND_INCLUDE_DIRS)
	unset(VALGRIND_PROGRAM)
endif()
