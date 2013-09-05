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

typedef struct _UriResolver
{
	// file -> fragments
	GHashTable *documents;
} UriResolver;

UriResolver* uri_resolver_new(void);
void uri_resolver_free(UriResolver *u);

char const *uri_resolver_add_document(UriResolver *u, char const *document);

// Add validator to the pool of known
// Return path in the document to the validator
char const *uri_resolver_add_validator(UriResolver *u,
                                       char const *document,
                                       char const *fragment,
                                       Validator *v);

Validator *uri_resolver_lookup_validator(UriResolver *u,
                                         char const *document,
                                         char const *fragment);

char const *uri_resolver_get_unresolved(UriResolver *u);

// Move everything except root fragment from the source to us.
bool uri_resolver_steal_documents(UriResolver *u, UriResolver *source);

char *uri_resolver_dump(UriResolver const *u);

#ifdef __cplusplus
}
#endif
