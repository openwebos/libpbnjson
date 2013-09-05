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

#include "validation_state.h"
#include "validator.h"

ValidationState *validation_state_new(Validator *validator,
                                      UriResolver *uri_resolver,
                                      Notification *notify)
{
	ValidationState *s = g_new(ValidationState, 1);
	if (!s)
		return s;
	validation_state_init(s, validator, uri_resolver, notify);
	return s;
}

void validation_state_free(ValidationState *s)
{
	validation_state_clear(s);
	g_free(s);
}

void validation_state_init(ValidationState *s,
                           Validator *validator,
                           UriResolver *uri_resolver,
                           Notification *notify)
{
	s->uri_resolver = uri_resolver;
	s->notify = notify;
	s->validator_stack = NULL;
	s->context_stack = NULL;

	validation_state_push_validator(s, validator);
}

void validation_state_clear(ValidationState *s)
{
	while (s->validator_stack)
		validation_state_pop_validator(s);
}

Validator *validation_state_get_validator(ValidationState *s)
{
	if (!s->validator_stack)
		return NULL;
	return (Validator *) s->validator_stack->data;
}

void validation_state_push_validator(ValidationState *s, Validator *v)
{
	s->validator_stack = g_slist_prepend(s->validator_stack, v);
	validator_init_state(v, s);
}

Validator *validation_state_pop_validator(ValidationState *s)
{
	if (!s->validator_stack)
		return NULL;
	GSList *old = s->validator_stack;
	s->validator_stack = g_slist_next(s->validator_stack);
	validator_cleanup_state((Validator *) old->data, s);
	g_slist_free_1(old);
	Validator *cur = validation_state_get_validator(s);
	validator_reactivate(cur, s);
	return cur;
}

void *validation_state_get_context(ValidationState *s)
{
	if (!s->context_stack)
		return NULL;
	return s->context_stack->data;
}

void validation_state_set_context(ValidationState *s, void *ctxt)
{
	s->context_stack->data = ctxt;
}

void validation_state_push_context(ValidationState *s, void *ctxt)
{
	s->context_stack = g_slist_prepend(s->context_stack, ctxt);
}

void *validation_state_pop_context(ValidationState *s)
{
	if (!s->context_stack)
		return NULL;
	GSList *old = s->context_stack;
	s->context_stack = g_slist_next(s->context_stack);
	void *ctxt = old->data;
	g_slist_free_1(old);
	return ctxt;
}

void validation_state_notify_error(ValidationState *s, ValidationErrorCode error, void *ctxt)
{
	if (!s->notify || !s->notify->error_func)
		return;
	s->notify->error_func(s, error, ctxt);
}

bool validation_state_have_default_properties(ValidationState *s)
{
	return s->notify && s->notify->default_property_func;
}

bool validation_state_issue_default_property(ValidationState *s,
                                             char const *key, jvalue_ref value,
                                             void *ctxt)
{
	return s->notify->default_property_func(s, key, value, ctxt);
}
