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

#include "validator.h"
#include "uri_scope.h"
#include "uri_resolver.h"
#include <assert.h>
#include <stdio.h>


void validator_init(Validator *v, ValidatorVtable *vtable)
{
	v->vtable = vtable;
}

Validator* validator_ref(Validator *v)
{
	if (!v)
		return NULL;
	assert(v->vtable);
	if (v->vtable->ref)
		return v->vtable->ref(v);
	return v;
}

void validator_unref(Validator *v)
{
	if (!v)
		return;
	assert(v->vtable);
	if (v->vtable->unref)
		v->vtable->unref(v);
}

bool validator_check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	assert(v && v->vtable && v->vtable->check);
	return v->vtable->check(v, e, s, ctxt);
}

bool validator_init_state(Validator *v, ValidationState *s)
{
	assert(v && v->vtable);
	if (!v->vtable->init_state)
		return true;
	return v->vtable->init_state(v, s);
}

void validator_cleanup_state(Validator *v, ValidationState *s)
{
	assert(v && v->vtable);
	if (!v->vtable->cleanup_state)
		return;
	return v->vtable->cleanup_state(v, s);
}

void validator_reactivate(Validator *v, ValidationState *s)
{
	if (!v)
		return;
	assert(v->vtable);
	if (!v->vtable->reactivate)
		return;
	v->vtable->reactivate(v, s);
}

Validator* validator_set_object_properties(Validator *v, ObjectProperties *p)
{
	assert(v && v->vtable);
	if (v->vtable->set_object_properties)
		return v->vtable->set_object_properties(v, p);
	return v;
}

Validator* validator_set_object_additional_properties(Validator *v, Validator *additional)
{
	assert(v && v->vtable);
	if (v->vtable->set_object_additional_properties)
		return v->vtable->set_object_additional_properties(v, additional);
	return v;
}

Validator* validator_set_object_required(Validator *v, ObjectRequired *p)
{
	assert(v && v->vtable);
	if (v->vtable->set_object_required)
		return v->vtable->set_object_required(v, p);
	return v;
}

Validator* validator_set_object_max_properties(Validator *v, size_t max)
{
	assert(v && v->vtable);
	if (v->vtable->set_object_max_properties)
		return v->vtable->set_object_max_properties(v, max);
	return v;
}

Validator* validator_set_object_min_properties(Validator *v, size_t min)
{
	assert(v && v->vtable);
	if (v->vtable->set_object_min_properties)
		return v->vtable->set_object_min_properties(v, min);
	return v;
}

Validator* validator_set_array_items(Validator *v, ArrayItems *a)
{
	assert(v && v->vtable);
	if (v->vtable->set_array_items)
		return v->vtable->set_array_items(v, a);
	return v;
}

Validator* validator_set_array_max_items(Validator *v, size_t maxItems)
{
	assert(v && v->vtable);
	if (v->vtable->set_array_max_items)
		return v->vtable->set_array_max_items(v, maxItems);
	return v;
}

Validator* validator_set_array_min_items(Validator *v, size_t minItems)
{
	assert(v && v->vtable);
	if (v->vtable->set_array_min_items)
		return v->vtable->set_array_min_items(v, minItems);
	return v;
}

Validator* validator_set_array_additional_items(Validator *v, Validator *additional)
{
	assert(v && v->vtable);
	if (v->vtable->set_array_additional_items)
		return v->vtable->set_array_additional_items(v, additional);
	return v;
}

Validator* validator_set_array_unique_items(Validator *v, bool unique)
{
	assert(v && v->vtable);
	if (v->vtable->set_array_unique_items)
		return v->vtable->set_array_unique_items(v, unique);
	return v;
}

Validator* validator_set_number_maximum(Validator *v, Number *n)
{
	assert(v && v->vtable);
	if (v->vtable->set_number_maximum)
		return v->vtable->set_number_maximum(v, n);
	return v;
}

Validator* validator_set_number_maximum_exclusive(Validator *v, bool exclusive)
{
	assert(v && v->vtable);
	if (v->vtable->set_number_maximum_exclusive)
		return v->vtable->set_number_maximum_exclusive(v, exclusive);
	return v;
}

void validator_visit(Validator *v,
                     VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                     void *ctxt)
{
	if (!v)
		return;
	assert(v->vtable);
	if (!v->vtable->visit)
		return;
	return v->vtable->visit(v, enter_func, exit_func, ctxt);
}

void VISITOR_ENTER_VOID(char const *key, Validator *v, void *ctxt)
{
}

void VISITOR_EXIT_VOID(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
}

void _validator_apply_features(char const *key, Validator *v, void *ctxt)
{
	assert(v && v->vtable);
	if (!v->vtable->apply_features)
		return;
	v->vtable->apply_features(key, v, ctxt);
}

void validator_apply_features(Validator *v)
{
	// Move features to the type_validator in the top-down descent.
	_validator_apply_features(ROOT_FRAGMENT, v, NULL);
	validator_visit(v, _validator_apply_features, VISITOR_EXIT_VOID, NULL);
}

void _validator_combine(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	assert(v && v->vtable);
	if (!v->vtable->combine_validators)
		return;
	return v->vtable->combine_validators(key, v, ctxt, new_v);
}

void validator_combine(Validator *v)
{
	validator_visit(v, VISITOR_ENTER_VOID, _validator_combine, NULL);
	_validator_combine(ROOT_FRAGMENT, v, NULL, NULL);
}

void _validator_finalize_parse(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	assert(v && v->vtable);
	if (!v->vtable->finalize_parse)
		return;
	return v->vtable->finalize_parse(key, v, ctxt, new_v);
}

Validator* validator_finalize_parse(Validator *v)
{
	validator_visit(v, VISITOR_ENTER_VOID, _validator_finalize_parse, NULL);
	Validator *new_v = NULL;
	_validator_finalize_parse(ROOT_FRAGMENT, v, NULL, &new_v);
	return new_v ? new_v : validator_ref(v);
}

void _validator_collect_uri_enter(char const *key, Validator *v, void *ctxt)
{
	assert(v && v->vtable);
	if (!v->vtable->collect_uri_enter)
		return;
	return v->vtable->collect_uri_enter(key, v, ctxt);
}

void _validator_collect_uri_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	if (!v)
		return;
	assert(v->vtable);
	if (!v->vtable->collect_uri_exit)
		return;
	return v->vtable->collect_uri_exit(key, v, ctxt, new_v);
}

void validator_collect_uri(Validator *v, char const *document, UriResolver *u)
{
	UriScope *uri_scope = uri_scope_new();
	uri_scope->uri_resolver = u;
	uri_scope_push_uri(uri_scope, document);

	_validator_collect_uri_enter(NULL, v, uri_scope);
	validator_visit(v, _validator_collect_uri_enter, _validator_collect_uri_exit, uri_scope);
	_validator_collect_uri_exit(NULL, v, uri_scope, NULL);

	uri_scope_pop_uri(uri_scope);
	uri_scope_free(uri_scope);
}

void _validator_dump_enter(char const *key, Validator *v, void *ctxt)
{
	assert(v && v->vtable);

	if (!v->vtable->dump_enter && !v->vtable->dump_exit)
	{
		if (key)
			fprintf((FILE *) ctxt, "%s:(", key);
		else
			fprintf((FILE *) ctxt, "(");
	}

	if (!v->vtable->dump_enter)
		return;
	return v->vtable->dump_enter(key, v, ctxt);
}

void _validator_dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	assert(v && v->vtable);

	if (!v->vtable->dump_enter && !v->vtable->dump_exit)
		fprintf((FILE *) ctxt, ")");

	if (!v->vtable->dump_exit)
		return;
	return v->vtable->dump_exit(key, v, ctxt, new_v);
}

void validator_dump(Validator *v, FILE *f)
{
	_validator_dump_enter(NULL, v, f);
	validator_visit(v, _validator_dump_enter, _validator_dump_exit, f);
	_validator_dump_exit(NULL, v, f, NULL);
}

Validator* validator_set_number_minimum(Validator *v, Number *n)
{
	assert(v && v->vtable);
	if (v->vtable->set_number_minimum)
		return v->vtable->set_number_minimum(v, n);
	return v;
}

Validator* validator_set_number_minimum_exclusive(Validator *v, bool exclusive)
{
	assert(v && v->vtable);
	if (v->vtable->set_number_minimum_exclusive)
		return v->vtable->set_number_minimum_exclusive(v, exclusive);
	return v;
}

Validator* validator_set_string_max_length(Validator *v, size_t maxLength)
{
	assert(v && v->vtable);
	if (v->vtable->set_string_max_length)
		return v->vtable->set_string_max_length(v, maxLength);
	return v;
}

Validator* validator_set_string_min_length(Validator *v, size_t minLength)
{
	assert(v && v->vtable);
	if (v->vtable->set_string_min_length)
		return v->vtable->set_string_min_length(v, minLength);
	return v;
}

Validator* validator_set_default(Validator *v, jvalue_ref def_value)
{
	assert(v && v->vtable);
	if (v->vtable->set_default)
		return v->vtable->set_default(v, def_value);
	return v;
}

jvalue_ref validator_get_default(Validator *v, ValidationState *s)
{
	assert(v && v->vtable);
	if (v->vtable->get_default)
		return v->vtable->get_default(v, s);
	return NULL;
}
