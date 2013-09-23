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

#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Validator Validator;

/** @brief Class UriResolver
 *
 * This class contains double maps of validators: document -> fragment -> validator.
 * Documents, fragments and validators are owned by the UriResolver, thus,
 * it must outlive the references. This is done to resolve cyclic dependencies
 * in the validator hierarchies (consider a reference down the tree pointing
 * to the root validator).
 */
typedef struct _UriResolver
{
	/** @brief Hash map of document -> hash table of fragments in the documents. */
	GHashTable *documents;
} UriResolver;

/** @brief Constructor */
UriResolver* uri_resolver_new(void);

/** @brief Destructor */
void uri_resolver_free(UriResolver *u);

/** @brief Remember document in the UriResolver.
 *
 * If there's a document with no fragments, it's considered unresolved,
 * because the root fragment # would be present otherwise.
 *
 * @param[in] u This object
 * @param[in] document Document to remember
 * @return Pointer to the document, contained in the hash table.
 */
char const *uri_resolver_add_document(UriResolver *u, char const *document);

/** @brief Add validator to the pool of known
 *
 * If there was no such document before, one is added implicitly
 * with uri_resolver_add_document().
 *
 * @param[in] u This object
 * @param[in] document Document the validator belongs to
 * @param[in] fragment Fragment in the document the validator belongs to
 * @param[in] v Validator to associate with these (document, fragment)
 * @return Pointer to the fragment in the hash map
 */
char const *uri_resolver_add_validator(UriResolver *u,
                                       char const *document,
                                       char const *fragment,
                                       Validator *v);

/** @brief Find validator that corresponds to the given (document, fragment).
 *
 * @param[in] u This object
 * @param[in] document Document to find ("" for local resolution)
 * @param[in] fragment Fragment to resolve ("#" for root scope)
 * @return Validator if found, NULL otherwise.
 */
Validator *uri_resolver_lookup_validator(UriResolver *u,
                                         char const *document,
                                         char const *fragment);

/** @brief Get one of the unresolved documents */
char const *uri_resolver_get_unresolved(UriResolver *u);

/** @brief Move everything except root fragment from the source to us. */
bool uri_resolver_steal_documents(UriResolver *u, UriResolver *source);

/** @brief Debug method */
char *uri_resolver_dump(UriResolver const *u);

#ifdef __cplusplus
}
#endif
