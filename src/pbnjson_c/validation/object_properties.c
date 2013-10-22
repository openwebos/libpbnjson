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

#include "object_properties.h"
#include "uri_resolver.h"
#include "validator.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void _validator_release(gpointer data)
{
	validator_unref((Validator *) data);
}

static void _release(Feature *f)
{
	ObjectProperties *o = (ObjectProperties *) f;
	g_hash_table_destroy(o->keys);
	g_free(o);
}

static bool _apply(Feature *f, Validator *v)
{
	ObjectProperties *o = (ObjectProperties *) f;
	validator_set_object_properties(v, o);
	return true;
}

static FeatureVtable object_properties_vtable =
{
	.release = _release,
	.apply = _apply,
};

ObjectProperties* object_properties_new(void)
{
	ObjectProperties *o = g_new0(ObjectProperties, 1);
	feature_init(&o->base, &object_properties_vtable);
	o->keys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _validator_release);
	return o;
}

ObjectProperties* object_properties_ref(ObjectProperties *o)
{
	return (ObjectProperties *) feature_ref(&o->base);
}

void object_properties_unref(ObjectProperties *o)
{
	feature_unref(&o->base);
}

void object_properties_add_key(ObjectProperties *o, char const *key, Validator *v)
{
	char *skey = g_strdup(key);
	g_hash_table_insert(o->keys, skey, v);
}

void object_properties_add_key_n(ObjectProperties *o, char const *key, size_t key_len, Validator *v)
{
	assert(o && o->keys);
	char *skey = g_strndup(key, key_len);
	g_hash_table_insert(o->keys, skey, v);
}

size_t object_properties_length(ObjectProperties *o)
{
	return g_hash_table_size(o->keys);
}

Validator* object_properties_lookup(ObjectProperties *o, char const *key)
{
	assert(o && o->keys);

	return (Validator *) g_hash_table_lookup(o->keys, key);
}

Validator* object_properties_lookup_n(ObjectProperties *o, char const *key, size_t key_len)
{
	// lookup validator by key
	// if not found, use generic validator
	char key_n[key_len + 1];
	strncpy(key_n, key, key_len);
	key_n[key_len] = 0;

	return object_properties_lookup(o, key_n);
}

void object_properties_visit(ObjectProperties *o,
                             VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                             void *ctxt)
{
	GHashTableIter it;
	g_hash_table_iter_init(&it, o->keys);
	char const *key = NULL;
	Validator *v = NULL;
	while (g_hash_table_iter_next(&it, (gpointer *) &key, (gpointer *) &v))
	{
		enter_func(key, v, ctxt);
		validator_visit(v, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(key, v, ctxt, &new_v);
		if (new_v)
			g_hash_table_iter_replace(&it, new_v);
	}
}

GHashTable *object_properties_gather_default(ObjectProperties *o, ValidationState *s)
{
	GHashTable *result = NULL;

	GHashTableIter it;
	g_hash_table_iter_init(&it, o->keys);
	char const *key = NULL;
	Validator *v = NULL;
	while (g_hash_table_iter_next(&it, (gpointer *) &key, (gpointer *) &v))
	{
		jvalue_ref def_value = validator_get_default(v, s);
		if (!def_value)
			continue;
		if (!result)
		{
			result = g_hash_table_new(g_str_hash, g_str_equal);
		}
		g_hash_table_insert(result, (gpointer) key, (gpointer) def_value);
	}
	return result;
}
