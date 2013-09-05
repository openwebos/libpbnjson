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

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Validator Validator;
typedef struct _UriResolver UriResolver;

typedef void (*JschemaErrorFunc)(size_t offset, char const *message, void *ctxt);


Validator* parse_schema(char const *str,
                        UriResolver *uri_resolver, char const *root_scope,
                        JschemaErrorFunc error_func, void *error_ctxt);

Validator* parse_schema_n(char const *str, size_t len,
                          UriResolver *uri_resolver, char const *root_scope,
                          JschemaErrorFunc error_func, void *error_ctxt);

// For unit tests
Validator* parse_schema_no_uri(char const *str,
                               JschemaErrorFunc error_func, void *error_ctxt);

Validator* parse_schema_bare(char const *str);

#ifdef __cplusplus
}
#endif
