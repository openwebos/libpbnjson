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

#include "uri_resolver.h"
#include "validator.h"
#include <assert.h>
#include <stdio.h>


static void _document_release(gpointer data)
{
	g_hash_table_destroy((GHashTable *) data);
}

UriResolver* uri_resolver_new(void)
{
	UriResolver *u = g_new0(UriResolver, 1);
	if (!u)
		return NULL;
	u->documents = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                     g_free, _document_release);
	if (!u->documents)
	{
		g_free(u);
		return NULL;
	}
	return u;
}

void uri_resolver_free(UriResolver *u)
{
	g_hash_table_destroy(u->documents);
	g_free(u);
}

static void _validator_release(gpointer data)
{
	validator_unref((Validator *) data);
}

char const *uri_resolver_add_document(UriResolver *u, char const *document)
{
	char const *orig_document = NULL;
	if (g_hash_table_lookup_extended(u->documents, document,
	                                 (gpointer *) &orig_document, NULL))
	{
		return orig_document;
	}

	GHashTable *fragments = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                              g_free, _validator_release);
	if (!fragments)
		return false;
	char *new_document = g_strdup(document);
	if (!new_document)
	{
		g_hash_table_destroy(fragments);
		return false;
	}
	g_hash_table_insert(u->documents, new_document, fragments);
	return new_document;
}

char const *_check_fragment(char const *fragment)
{
	if (!fragment || !fragment[0])
		return "#";
	return fragment;
}

char const *uri_resolver_add_validator(UriResolver *u,
                                       char const *document,
                                       char const *fragment,
                                       Validator *v)
{
	assert(u);

	GHashTable *fragments = g_hash_table_lookup(u->documents, document);
	if (!fragments)
	{
		uri_resolver_add_document(u, document);
		fragments = g_hash_table_lookup(u->documents, document);
	}
	assert(fragments && "The table must be added earlier with uri_resolver_add_document()");

	if (g_hash_table_lookup(fragments, fragment))
		fprintf(stderr, "The same fragment %s is added second time!\n", fragment); // Look for bugs!

	char *new_fragment = g_strdup(fragment);
	if (!new_fragment)
		return NULL;

	g_hash_table_insert(fragments, new_fragment, validator_ref(v));
	return new_fragment;
}

Validator *uri_resolver_lookup_validator(UriResolver *u,
                                         char const *document,
                                         char const *fragment)
{
	assert(u);

	GHashTable *fragments = g_hash_table_lookup(u->documents, document);
	if (!fragments)
		return NULL;
	Validator *v = g_hash_table_lookup(fragments, fragment);
	return v;
}

static gint _compare_key(gconstpointer a, gconstpointer b)
{
	return strcmp((char const *) a, (char const *) b);
}

char const *uri_resolver_get_unresolved(UriResolver *u)
{
	assert(u);

	GHashTableIter it;
	g_hash_table_iter_init(&it, u->documents);
	char const *document = NULL;
	GHashTable *fragments = NULL;
	while (g_hash_table_iter_next(&it, (gpointer *) &document, (gpointer *) &fragments))
	{
		// If there's an empty fragment hash map, it's what's to be resolved.
		if (!fragments || !g_hash_table_size(fragments))
			return document;
	}
	return NULL;
}

bool uri_resolver_steal_documents(UriResolver *u, UriResolver *source)
{
	if (!source)
		return true;

	GHashTableIter it;
	g_hash_table_iter_init(&it, source->documents);
	char *document = NULL;
	gpointer fragments = NULL;
	while (g_hash_table_iter_next(&it, (gpointer *) &document, &fragments))
	{
		// Skip the root fragment
		if (!*document)
			continue;

		gpointer old_fragments = g_hash_table_lookup(u->documents, document);
		if (old_fragments)
		{
			// FIXME: The document has been already resolved?
			return false;
		}

		g_hash_table_iter_steal(&it);
		g_hash_table_insert(u->documents, document, fragments);
	}
	return true;
}

char *uri_resolver_dump(UriResolver const *u)
{
	char *result = NULL;
	if (!u)
		return result;

	result = g_strdup("");

	GHashTableIter it1;
	g_hash_table_iter_init(&it1, u->documents);
	char const *document = NULL;
	GHashTable *fragments = NULL;
	while (g_hash_table_iter_next(&it1, (void **) &document, (void **) &fragments))
	{
		char *sd = g_strconcat(document, ":", NULL);
		if (fragments)
		{
			GList *keys = g_hash_table_get_keys(fragments);
			keys = g_list_sort(keys, _compare_key);
			GList *head = keys;
			while (head)
			{
				char *sf = g_strconcat(sd, " ", head->data, NULL);
				g_free(sd);
				sd = sf;
				head = g_list_next(head);
			}
			g_list_free(keys);
		}
		char *r2 = g_strconcat(result, sd, "\n", NULL);
		g_free(sd);
		g_free(result);
		result = r2;
	}
	return result;
}
