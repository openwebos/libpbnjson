// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
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

#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _UriResolver UriResolver;

/** @brief URI scope stack class */
typedef struct _UriScope
{
	UriResolver *uri_resolver;  /**< @brief UriResolver for convenience */

	/** @brief Stack contains pointers to document URI. */
	GSList *uri_stack;
} UriScope;

/** @brief Constructor */
UriScope *uri_scope_new(void);

/** @brief Destructor */
void uri_scope_free(UriScope *u);

/** @brief Get length of the top document as a hint for memory allocation. */
int uri_scope_get_document_length(UriScope const *u);

/** @brief Copy document into the buffer
 *
 * @param[in] u This object
 * @param[in] buffer The address of the buffer to store the document
 * @param[in] chars_required The size of the buffer (obtained from uri_scope_get_document_length())
 * @return Pointer that is equal to the buffer.
 */
char const *uri_scope_get_document(UriScope const *u, char *buffer, int chars_required);

/** @brief Get current fragment from the top of the stack. */
char const *uri_scope_get_fragment(UriScope const *u);

/** @brief Push new uri == (document, fragment), resolve it against the top of the stack. */
bool uri_scope_push_uri(UriScope *u, char const *uri);

/** @brief Pop (document, fragment) from the stacks. */
void uri_scope_pop_uri(UriScope *u);

/** @brief Escape special characters (like '/') in JSON pointer.
 *
 * @param[in] fragment Input string (for instance, "#/definitions/a)
 * @param[in] fragment_len Length of input string
 * @param[in] buffer Where to place the result, must be twice as large as the fragment
 * @return Pointer that is equal to buffer.
 */
char const *escape_json_pointer(char const *fragment, size_t fragment_len, char *buffer);

/** @brief Unescape special characters (like '/') in JSON pointer.
 *
 * @param[in] fragment Input data
 * @param[in] buffer Where to place the result. Must be as long as the input string.
 * @return Pointer that is equal to buffer.
 */
char const *unescape_json_pointer(char const *fragment, char *buffer);

/** @brief ROOT_FRAGMENT = "#". Is used when gathering local URI. */
extern char const *const ROOT_FRAGMENT;

/** @brief ROOT_DEFINITIONS = "#/definitions". Is used when gathering local URI. */
extern char const *const ROOT_DEFINITIONS;

#ifdef __cplusplus
}
#endif
