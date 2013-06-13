// @@@LICENSE
//
//      Copyright (c) 2009-2013 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#cmakedefine HAVE_EXECINFO_H 1

#cmakedefine HAVE_CXXABI_H_ 1

#if HAVE_EXECINFO_H
	#include <execinfo.h>
#endif

#if HAVE_CXXABI_H_
	#define DEMANGLE_CPP 1

	#include <cxxabi.h>
#ifdef __cplusplus
	using namespace abi;
#endif
#else
	#undef DEMANGLE_CPP
#endif
