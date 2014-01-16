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

#include "combined_validator.h"
#include "generic_validator.h"
#include "validation_state.h"
#include "validation_event.h"
#include "validation_api.h"
#include <jobject.h>

typedef struct _MyContext
{
	GList *states;
	Notification *notify;
} MyContext;

static bool _all_of_check(ValidationEvent const *e,
                          ValidationState *real_state,
                          void *ctxt,
                          bool *all_finished)
{
	MyContext *my_ctxt = (MyContext *) validation_state_get_context(real_state);

	*all_finished = true;
	GList *it = my_ctxt->states;
	while (it)
	{
		ValidationState *s = it->data;
		if (!validation_check(e, s, ctxt))
		{
			// context notifications used only if inner errors are suppressed
			if (my_ctxt->notify)
				validation_state_notify_error(real_state, VEC_NOT_EVERY_ALL_OF, ctxt);
			return false;
		}

		if (s->validator_stack)
			*all_finished = false;

		it = g_list_next(it);
	}

	return true;
}

static bool any_of_check_with_error(ValidationEvent const *e,
                                    ValidationState *real_state,
                                    void *ctxt,
                                    bool *all_finished,
                                    ValidationErrorCode error)
{
	MyContext *my_ctxt = (MyContext *) validation_state_get_context(real_state);

	*all_finished = true;
	GList *it = my_ctxt->states;
	bool res = false;
	while (it)
	{
		ValidationState *s = it->data;
		if (validation_check(e, s, ctxt))
		{
			res = true;
			if (!s->validator_stack)
			{
				*all_finished = true;
				return true;
			}
		}
		else
		{
			// If there wasn't a match, no need to keep the validator
			// in the list of active ones.
			GList *next = g_list_next(it);
			my_ctxt->states = g_list_remove_link(my_ctxt->states, it);
			validation_state_free(s);
			g_list_free_1(it);
			it = next;
			continue;
		}

		if (s->validator_stack)
			*all_finished = false;

		it = g_list_next(it);
	}

	if (*all_finished && !res)
	{
		// context notifications used only if inner errors are suppressed
		if (my_ctxt->notify)
			validation_state_notify_error(real_state, error, ctxt);
	}

	return res;
}

static bool any_of_check(ValidationEvent const *e,
                         ValidationState *s,
                         void *ctxt,
                         bool *all_finished)
{
	return any_of_check_with_error(e, s, ctxt, all_finished, VEC_NEITHER_OF_ANY);
}

static bool enum_check(ValidationEvent const *e,
                       ValidationState *s,
                       void *ctxt,
                       bool *all_finished)
{
	return any_of_check_with_error(e, s, ctxt, all_finished, VEC_UNEXPECTED_VALUE);
}

static bool _one_of_check(ValidationEvent const *e,
                          ValidationState *real_state,
                          void *ctxt,
                          bool *all_finished)
{
	MyContext *my_ctxt = (MyContext *) validation_state_get_context(real_state);

	*all_finished = true;
	bool one_succeeded = false;

	GList *it = my_ctxt->states;
	while (it)
	{
		ValidationState *s = it->data;
		if (validation_check(e, s, ctxt))
		{
			if (!s->validator_stack)
			{
				if (one_succeeded)
				{
					// context notifications used only if inner errors are suppressed
					if (my_ctxt->notify)
						validation_state_notify_error(real_state, VEC_MORE_THAN_ONE_OF, ctxt);
					*all_finished = true;
					return false;
				}
				one_succeeded = true;
			}
		}
		else
		{
			// If there wasn't a match, no need to keep the validator
			// in the list of active ones.
			GList *next = g_list_next(it);
			my_ctxt->states = g_list_remove_link(my_ctxt->states, it);
			validation_state_free(s);
			g_list_free_1(it);
			it = next;
			continue;
		}

		if (s->validator_stack)
			*all_finished = false;

		it = g_list_next(it);
	}

	if (!my_ctxt->states)
	{
		// context notifications used only if inner errors are suppressed
		if (my_ctxt->notify)
			validation_state_notify_error(real_state, VEC_NEITHER_OF_ANY, ctxt);
		*all_finished = true;
		return false;
	}

	if (one_succeeded)
	{
		*all_finished = true;
		return true;
	}

	// Still expecting more input.
	*all_finished = false;
	return true;
}

static bool not_check(ValidationEvent const *e,
                      ValidationState *real_state,
                      void *ctxt,
                      bool *all_finished)
{
	MyContext *my_ctxt = (MyContext *) validation_state_get_context(real_state);

	*all_finished = true;
	GList *it = my_ctxt->states;
	while (it)
	{
		ValidationState *s = it->data;
		// if all validations of inner validator passed
		if (validation_check(e, s, ctxt) && !s->validator_stack)
		{
			// context notifications used only if inner errors are suppressed
			if (my_ctxt->notify)
				validation_state_notify_error(real_state, VEC_SOME_OF_NOT, ctxt);
			return false;
		}

		if (s->validator_stack)
		{
			*all_finished = false;
			it = g_list_next(it);
			continue;
		}

		// If there wasn't a match, no need to keep the validator
		// in the list of active ones.
		GList *next = g_list_next(it);
		my_ctxt->states = g_list_remove_link(my_ctxt->states, it);
		validation_state_free(s);
		g_list_free_1(it);
		it = next;
	}

	return true;
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	CombinedValidator *vcomb = (CombinedValidator *) v;

	bool all_finished;
	bool res = vcomb->check_all(e, s, c, &all_finished);
	if (!res || all_finished)
	{
		validation_state_pop_validator(s);
	}

	return res;
}

static void init_states_with_notify(Validator *v, ValidationState *s, MyContext *my_ctxt, Notification* notify)
{
	CombinedValidator *vcomb = (CombinedValidator *) v;
	GSList *it = vcomb->validators;
	while (it)
	{
		ValidationState *substate = validation_state_new(it->data, s->uri_resolver, notify);
		my_ctxt->states = g_list_append(my_ctxt->states, substate);
		it = g_slist_next(it);
	}
}

static bool init_state_collect_errors(Validator *v, ValidationState *s)
{
	MyContext *my_ctxt = g_slice_new0(MyContext);
	init_states_with_notify(v, s, my_ctxt, s->notify);
	validation_state_push_context(s, my_ctxt);
	return true;
}

static bool init_state_suppress_errors(Validator *v, ValidationState *s)
{
	Notification *notify = NULL;
	if (s->notify)
	{
		notify = g_slice_new0(Notification);
		notify->default_property_func = s->notify->default_property_func;
		notify->has_array_duplicates = s->notify->has_array_duplicates;
	}

	MyContext *my_ctxt = g_slice_new0(MyContext);
	my_ctxt->notify = notify;
	init_states_with_notify(v, s, my_ctxt, notify);
	validation_state_push_context(s, my_ctxt);
	return true;
}

static void cleanup_state(Validator *v, ValidationState *s)
{
	MyContext *c = validation_state_pop_context(s);
	g_list_free_full(c->states, (GDestroyNotify) validation_state_free);
	g_slice_free(Notification, c->notify);
	g_slice_free(MyContext, c);
}

static Validator* ref(Validator *validator)
{
	CombinedValidator *v = (CombinedValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	CombinedValidator *v = (CombinedValidator *) validator;
	if (--v->ref_count)
		return;
	combined_validator_release(v);
}

static Validator* set_default(Validator *validator, jvalue_ref def_value)
{
	CombinedValidator *v = (CombinedValidator *) validator;
	j_release(&v->def_value);
	v->def_value = jvalue_copy(def_value);
	return validator;
}

static jvalue_ref get_default(Validator *validator, ValidationState *s)
{
	CombinedValidator *v = (CombinedValidator *) validator;
	return v->def_value;
}

static void _visit(Validator *v,
                   VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                   void *ctxt)
{
	CombinedValidator *a = (CombinedValidator *) v;
	GSList *iter = a->validators;
	while (iter)
	{
		Validator *v = iter->data;
		if (!v)
			continue;
		enter_func(NULL, v, ctxt);
		validator_visit(v, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, v, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(v);
			iter->data = new_v;
		}
		iter = g_slist_next(iter);
	}
}

static void _dump_enter(char const *key, Validator *v, void *ctxt)
{
	//CombinedValidator *c = (CombinedValidator *) v;
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	fprintf((FILE *) ctxt, "<");
}

static void _dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	fprintf((FILE *) ctxt, ">");
}

ValidatorVtable combined_vtable =
{
	.check = _check,
	.init_state = init_state_suppress_errors,
	.cleanup_state = cleanup_state,
	.ref = ref,
	.unref = unref,
	.visit = _visit,
	.set_default = set_default,
	.get_default = get_default,
	.dump_enter = _dump_enter,
	.dump_exit = _dump_exit,
};

CombinedValidator* combined_validator_new(void)
{
	CombinedValidator *self = g_new0(CombinedValidator, 1);
	self->ref_count = 1;
	validator_init(&self->base, &combined_vtable);
	return self;
}

CombinedValidator* all_of_validator_new(void)
{
	CombinedValidator *self = combined_validator_new();
	combined_validator_convert_to_all_of(self);
	return self;
}

CombinedValidator* any_of_validator_new(void)
{
	CombinedValidator *self = combined_validator_new();
	combined_validator_convert_to_any_of(self);
	return self;
}

CombinedValidator* one_of_validator_new(void)
{
	CombinedValidator *self = combined_validator_new();
	combined_validator_convert_to_one_of(self);
	return self;
}

CombinedValidator* not_validator_new(void)
{
	CombinedValidator *self = combined_validator_new();
	combined_validator_convert_to_not(self);
	return self;
}

CombinedValidator* enum_validator_new(void)
{
	CombinedValidator *self = combined_validator_new();
	combined_validator_convert_to_enum(self);
	return self;
}

void combined_validator_convert_to_all_of(CombinedValidator *v)
{
	v->check_all = _all_of_check;
}

void combined_validator_convert_to_any_of(CombinedValidator *v)
{
	v->check_all = any_of_check;
}

void combined_validator_convert_to_one_of(CombinedValidator *v)
{
	v->check_all = _one_of_check;
}

void combined_validator_convert_to_not(CombinedValidator *v)
{
	v->check_all = not_check;
	// Add inverse generic validator to track JSON objects/arrays depth
	combined_validator_add_value(v, inverse_generic_validator_instance());
}

static bool enum_check_add_value(CombinedValidator *c, Validator *v)
{
	GSList *it = c->validators;
	while (it)
	{
		if (validator_equals(v, it->data))
			return false;
		it = g_slist_next(it);
	}
	return true;
}

void combined_validator_convert_to_enum(CombinedValidator *v)
{
	v->check_all = enum_check;
}

void combined_validator_collect_errors(CombinedValidator *v)
{
	v->base.vtable->init_state = init_state_collect_errors;
}

void combined_validator_suppress_errors(CombinedValidator *v)
{
	v->base.vtable->init_state = init_state_suppress_errors;
}


static void _validator_release(gpointer data)
{
	validator_unref((Validator *) data);
}

void combined_validator_release(CombinedValidator *v)
{
	g_slist_free_full(v->validators, _validator_release);
	j_release(&v->def_value);
	g_free(v);
}

void combined_validator_add_value(CombinedValidator *a, Validator *v)
{
	a->validators = g_slist_prepend(a->validators, v);
}

bool combined_validator_add_enum_value(CombinedValidator *a, Validator *v)
{
	if (!enum_check_add_value(a, v))
	{
		validator_unref(v);
		return false;
	}

	combined_validator_add_value(a, v);
	return true;
}
