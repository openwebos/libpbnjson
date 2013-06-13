// @@@LICENSE
//
//      Copyright (c) 2013 LG Electronics, Inc.
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

#ifndef YAJL_COMPAT_H_
#define YAJL_COMPAT_H_

#ifdef HAVE_YAJL_VERSION_H
#include <yajl/yajl_version.h>
#else
/* dummy version lower than 20000 */
#define YAJL_VERSION 10000
#endif

#if YAJL_VERSION < 20000
typedef unsigned int yajl_size_t;
#else
typedef size_t yajl_size_t;
#endif

#endif /* YAJL_COMPAT_H_ */
