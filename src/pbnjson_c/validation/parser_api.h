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

/** @brief Schema parsing error callback.
 *
 * The parser may notify about schema parsing errors.
 * @param[in] offset The byte where the error was detected.
 * @param[in] message Error description either from YAJL or from the schema analyzer.
 * @param[in] ctxt The pointer user passed to the parser.
 */
typedef void (*JschemaErrorFunc)(size_t offset, char const *message, void *ctxt);


/** @brief Parse JSON schema, and create corresponding validator for it.
 *
 * @param[in] str Null-terminated string containing schema definitions.
 * @param[in] uri_resolver Map of the validators, which may be referred later.
 * @param[in] root_scope Root URI scope of current schema. If it's a file,
 *      file:///full/path/to/this/schema may be given to let the parser
 *      track URI scope. If empty, URI scope isn't tracked.
 * @param[in] error_func The callback to notify the about schema parsing error.
 * @param[in] error_ctxt Pointer to any data to be passed to the error_func.
 * @return Validator or NULL.
 */
Validator* parse_schema(char const *str,
                        UriResolver *uri_resolver, char const *root_scope,
                        JschemaErrorFunc error_func, void *error_ctxt);

/** @brief Parse JSON schema, and create corresponding validator for it.
 *
 * @param[in] str Pointer to the memory containing JSON schema.
 * @param[in] len Schema length in bytes.
 * @param[in] uri_resolver Map of the validators, which may be referred later.
 * @param[in] root_scope Root URI scope of current schema. If it's a file,
 *      file:///full/path/to/this/schema may be given to let the parser
 *      track URI scope. If empty, URI scope isn't tracked.
 * @param[in] error_func The callback to notify the about schema parsing error.
 * @param[in] error_ctxt Pointer to any data to be passed to the error_func.
 * @return Validator or NULL.
 */
Validator* parse_schema_n(char const *str, size_t len,
                          UriResolver *uri_resolver, char const *root_scope,
                          JschemaErrorFunc error_func, void *error_ctxt);

/** @brief Simplified parser interface for unit tests. */
Validator* parse_schema_no_uri(char const *str,
                               JschemaErrorFunc error_func, void *error_ctxt);

/** @brief Optimistic parser interface for unit tests. */
Validator* parse_schema_bare(char const *str);

#ifdef __cplusplus
}
#endif
