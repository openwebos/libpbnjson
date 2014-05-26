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

#include "definitions.h"
#include "parser_context.h"
#include "uri_scope.h"
#include <assert.h>


typedef struct _NameValidator
{
	char *name;
	Validator *validator;
} NameValidator;

static void _release_definition(gpointer d)
{
	NameValidator *nv = (NameValidator *) d;
	g_free(nv->name);
	validator_unref(nv->validator);
	g_free(nv);
}

static Validator* ref(Validator *validator)
{
	Definitions *d = (Definitions *) validator;
	++d->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	Definitions *d = (Definitions *) validator;
	if (!d || --d->ref_count)
		return;
	g_slist_free_full(d->validators, _release_definition);
	g_free(d->name);
	g_free(d);
}

static void _visit(Validator *v,
                   VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                   void *ctxt)
{
	Definitions *d = (Definitions *) v;
	if (!d || !d->validators)
		return;

	enter_func("definitions", v, ctxt);

	GSList *next = d->validators;
	while (next)
	{
		NameValidator *nv = (NameValidator *) next->data;

		enter_func(nv->name, nv->validator, ctxt);
		validator_visit(nv->validator, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(nv->name, nv->validator, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(nv->validator);
			nv->validator = new_v;
		}

		next = g_slist_next(next);
	}

	exit_func("definitions", v, ctxt, NULL);
}

static void _collect_uri_enter(char const *key, Validator *v, void *ctxt)
{
	//Definitions *d = (Definitions *) v;
	UriScope *uri_scope = (UriScope *) ctxt;

	char escaped_key[strlen(key)*2 + 1];
	uri_scope_push_fragment_leaf(uri_scope, escape_json_pointer(key, escaped_key));
}

static void _collect_uri_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	//Definitions *d = (Definitions *) v;
	UriScope *uri_scope = (UriScope *) ctxt;

	uri_scope_pop_fragment_leaf(uri_scope);
}

static ValidatorVtable definitions_vtable =
{
	.ref = ref,
	.unref = unref,
	.visit = _visit,
	.collect_uri_enter = _collect_uri_enter,
	.collect_uri_exit = _collect_uri_exit,
};

Definitions* definitions_new(void)
{
	Definitions *d = g_new0(Definitions, 1);
	d->ref_count = 1;
	validator_init(&d->base, &definitions_vtable);
	return d;
}

void definitions_unref(Definitions *d)
{
	validator_unref(&d->base);
}

void definitions_set_name(Definitions *d, StringSpan *name)
{
	d->name = g_strndup(name->str, name->str_len);
}

void definitions_add(Definitions *d, StringSpan *name, Validator *v)
{
	NameValidator *nv = g_new0(NameValidator, 1);
	nv->name = g_strndup(name->str, name->str_len);
	nv->validator = v;
	d->validators = g_slist_prepend(d->validators, nv);
}

void definitions_collect_schemas(Definitions *d, UriScope *uri_scope)
{
	assert(d);
	assert(uri_scope);
	if (!d->validators)
		return;

	GSList *next = d->validators;
	uri_scope_push_fragment_leaf(uri_scope, "definitions");
	while (next)
	{
		NameValidator *nv = (NameValidator *) next->data;
		Validator *v = nv->validator;
		assert(v->vtable->collect_schemas); // we know thata all top validators under definitions should be SchemaParsing
		uri_scope_push_fragment_leaf(uri_scope, nv->name);
		_validator_collect_schemas(v, uri_scope);
		uri_scope_pop_fragment_leaf(uri_scope);

		next = g_slist_next(next);
	}
	uri_scope_pop_fragment_leaf(uri_scope);
}
