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

#include "reference.h"
#include "parser_context.h"
#include "validation_state.h"
#include "uri_resolver.h"
#include "uri_scope.h"
#include <glib.h>
#include <assert.h>
#include <stdio.h>


static void _release(Validator *v)
{
	Reference *r = (Reference *) v;
	g_free(r->target);
	g_free(r->fragment);
	g_free(r);
}

static Validator *_resolve_reference(Reference *r, UriResolver *uri_resolver)
{
	if (!r->validator)
	{
		r->validator = uri_resolver_lookup_validator(uri_resolver,
		                                             r->document, r->fragment);
		if (!r->validator)
		{
			fprintf(stderr, "Can't resolve %s %s\n", r->document, r->fragment);
			return NULL;
		}
	}
	return r->validator;
}

static jvalue_ref _get_default(Validator *v, ValidationState *s)
{
	if (!s->uri_resolver)
		return v->def_value;
	Reference *r = (Reference *) v;
	if (!_resolve_reference(r, s->uri_resolver))
		return v->def_value;
	return validator_get_default(r->validator, s);
}

static bool _init_state(Validator *v, ValidationState *s)
{
	if (!s->uri_resolver)
		return false;
	Reference *r = (Reference *) v;
	if (!_resolve_reference(r, s->uri_resolver))
		return false;
	validation_state_push_validator(s, r->validator);
	return true;
}

static void _reactivate(Validator *v, ValidationState *s)
{
	validation_state_pop_validator(s);
}

static void _collect_uri_enter(char const *key, Validator *v, void *ctxt)
{
	Reference *r = (Reference *) v;
	UriScope *uri_scope = (UriScope *) ctxt;

	// Push target to resolve it in the current context.
	uri_scope_push_uri(uri_scope, r->target);

	int chars_required = uri_scope_get_document_length(uri_scope);
	char buffer[chars_required];
	char const *document = uri_scope_get_document(uri_scope, buffer, chars_required);

	assert(!r->document && !r->fragment && "Reference URI should be collected only once");
	// Remember document, which will be owned by the uri_resolver
	r->document = uri_resolver_add_document(uri_scope->uri_resolver, document);
	// The fragment will be our property
	r->fragment = uri_scope_steal_fragment(uri_scope);

	// Return to the previous URI context
	uri_scope_pop_uri(uri_scope);
}

static void _collect_uri_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	//Reference *r = (Reference *) v;
	//UriScope *uri_scope = (UriScope *) ctxt;
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	Reference *r = (Reference *) v;
	fprintf(stderr, "Reference hasn't been resolved (or used directly?): %s %s\n",
	        r->document, r->fragment);
	// If the reference hasn't been resolved, it can't validate anything.
	return false;
}

static void _dump_enter(char const *key, Validator *v, void *ctxt)
{
	Reference *r = (Reference *) v;
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	if (r->validator)
	{
		fprintf((FILE *) ctxt, "$");
		validator_dump(r->validator, (FILE *) ctxt);
	}
	else
		fprintf((FILE *) ctxt, "($%s %s)", r->document, r->fragment);
}

static ValidatorVtable reference_vtable =
{
	.release = _release,
	.get_default = _get_default,
	.init_state = _init_state,
	.reactivate = _reactivate,
	.check = _check,
	.collect_uri_enter = _collect_uri_enter,
	.collect_uri_exit = _collect_uri_exit,
	.dump_enter = _dump_enter,
};

Reference *reference_new(void)
{
	Reference *r = g_new0(Reference, 1);
	if (!r)
		return NULL;
	validator_init(&r->base, &reference_vtable);
	return r;
}

Reference *reference_ref(Reference *r)
{
	if (r)
		validator_ref(&r->base);
	return r;
}

void reference_unref(Reference *r)
{
	if (r)
		validator_unref(&r->base);
}

bool reference_set_target(Reference *r, StringSpan *target)
{
	if (!r)
		return true;
	char *t = g_strndup(target->str, target->str_len);
	if (!t)
		return false;
	g_free(r->target);
	r->target = t;
	return true;
}
