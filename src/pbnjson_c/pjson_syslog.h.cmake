// @@@LICENSE
//
//      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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

// If the macro is defined to override output, then
// we disable the other targets

#cmakedefine HAVE_SYSLOG_H 1
#ifndef PJSON_LOG_STDOUT
#cmakedefine HAVE_VSYSLOG 1
#endif
#cmakedefine HAVE_VFPRINTF 1

#if HAVE_SYSLOG_H
#include <syslog.h>
#endif
