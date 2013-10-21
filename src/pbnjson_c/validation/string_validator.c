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

#include "string_validator.h"
#include "validation_state.h"
#include "validation_event.h"
#include "parser_context.h"
#include <jobject.h>
#include <glib.h>
#include <string.h>


static bool _check_conditions(StringValidator *v, ValidationEvent const *e,
                              ValidationState *s, void *c)
{
	if (e->type != EV_STR)
	{
		validation_state_notify_error(s, VEC_NOT_STRING, c);
		return false;
	}

	if (v->expected_value &&
	    strncmp(v->expected_value, e->value.string.ptr, e->value.string.len))
	{
		validation_state_notify_error(s, VEC_UNEXPECTED_VALUE, c);
		return false;
	}

	if (v->min_length >= 0 &&
	    e->value.string.len < v->min_length)
	{
		validation_state_notify_error(s, VEC_STRING_TOO_SHORT, c);
		return false;
	}

	if (v->max_length >= 0 &&
	    e->value.string.len > v->max_length)
	{
		validation_state_notify_error(s, VEC_STRING_TOO_LONG, c);
		return false;
	}

	return true;
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	StringValidator *vstr = (StringValidator *) v;
	bool res = _check_conditions(vstr, e, s, c);
	validation_state_pop_validator(s);
	return res;
}

static Validator* ref(Validator *validator)
{
	StringValidator *v = (StringValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	StringValidator *v = (StringValidator *) validator;
	if (--v->ref_count)
		return;
	string_validator_release(v);
}

static void _set_max_length(Validator *v, size_t max_length)
{
	StringValidator *s = (StringValidator *) v;
	string_validator_add_max_length_constraint(s, max_length);
}

static void _set_min_length(Validator *v, size_t min_length)
{
	StringValidator *s = (StringValidator *) v;
	string_validator_add_min_length_constraint(s, min_length);
}

static void set_default(Validator *validator, jvalue_ref def_value)
{
	StringValidator *v = (StringValidator *) validator;
	j_release(&v->def_value);
	v->def_value = jvalue_copy(def_value);
}

static jvalue_ref get_default(Validator *validator, ValidationState *s)
{
	StringValidator *v = (StringValidator *) validator;
	return v->def_value;
}

static void _dump_enter(char const *key, Validator *v, void *ctxt)
{
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	fprintf((FILE *) ctxt, "(s)");
}

static ValidatorVtable string_vtable =
{
	.check = _check,
	.ref = ref,
	.unref = unref,
	.set_string_max_length = _set_max_length,
	.set_string_min_length = _set_min_length,
	.set_default = set_default,
	.get_default = get_default,
	.dump_enter = _dump_enter,
};

StringValidator* string_validator_new(void)
{
	StringValidator *self = g_new0(StringValidator, 1);
	if (!self)
		return NULL;
	self->ref_count = 1;
	validator_init(&self->base, &string_vtable);
	self->min_length = -1;
	self->max_length = -1;
	return self;
}

void string_validator_release(StringValidator *v)
{
	g_free(v->expected_value);
	j_release(&v->def_value);
	g_free(v);
}

void string_validator_add_min_length_constraint(StringValidator *v, size_t min_length)
{
	v->min_length = min_length;
}

void string_validator_add_max_length_constraint(StringValidator *v, size_t max_length)
{
	v->max_length = max_length;
}

bool string_validator_add_expected_value(StringValidator *v, StringSpan *span)
{
	g_free(v->expected_value);
	v->expected_value = g_strndup(span->str, span->str_len);
	return v->expected_value != NULL;
}
