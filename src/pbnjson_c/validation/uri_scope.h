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

typedef struct _UriResolver UriResolver;

typedef struct _UriScope
{
	UriResolver *uri_resolver;

	// Stack contains pointers to document URI.
	GSList *uri_stack;
	// Fragment is char *, keeping NULL-terminated string
	// of current fragment.
	GSList *fragment_stack;
} UriScope;

UriScope *uri_scope_new(void);
void uri_scope_free(UriScope *u);

int uri_scope_get_document_length(UriScope const *u);
char const *uri_scope_get_document(UriScope const *u, char *buffer, int chars_required);
char const *uri_scope_get_fragment(UriScope const *u);
char *uri_scope_steal_fragment(UriScope *u);

bool uri_scope_push_uri(UriScope *u, char const *uri);
void uri_scope_pop_uri(UriScope *u);

char const *uri_scope_push_fragment_leaf(UriScope *u, char const *leaf);
char const *uri_scope_pop_fragment_leaf(UriScope *u);

char const *escape_json_pointer(char const *fragment, char *buffer);
char const *unescape_json_pointer(char const *fragment, char *buffer);

extern char const *const ROOT_FRAGMENT;
extern char const *const ROOT_DEFINITIONS;

#ifdef __cplusplus
}
#endif
