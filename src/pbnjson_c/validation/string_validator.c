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

#include "string_validator.h"
#include "pattern.h"
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
	    (strlen(v->expected_value) != e->value.string.len
	    || strncmp(v->expected_value, e->value.string.ptr, e->value.string.len)))
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

	if (v->pattern)
	{
		size_t len = e->value.string.len;
		char buf[len + 1];
		memcpy(buf, e->value.string.ptr, len);
		buf[len] = 0;
		bool res = g_regex_match(v->pattern, buf, 0, NULL);
		if (!res)
		{
			validation_state_notify_error(s, VEC_STRING_NOT_PATTERN, c);
			return false;
		}
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

static bool check_generic(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	validation_state_pop_validator(s);
	if (e->type != EV_STR)
	{
		validation_state_notify_error(s, VEC_NOT_STRING, ctxt);
		return false;
	}
	return true;
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

static Validator* set_max_length(Validator *v, size_t max_length)
{
	StringValidator *s = (StringValidator *) v;
	string_validator_add_max_length_constraint(s, max_length);
	return v;
}

static Validator* set_min_length(Validator *v, size_t min_length)
{
	StringValidator *s = (StringValidator *) v;
	string_validator_add_min_length_constraint(s, min_length);
	return v;
}

static Validator* set_pattern(Validator *v, Pattern *p)
{
	StringValidator *s = (StringValidator *) v;
	string_validator_set_pattern(s, p->regex);
	return v;
}

static Validator* set_default(Validator *validator, jvalue_ref def_value)
{
	StringValidator *v = (StringValidator *) validator;
	j_release(&v->def_value);
	v->def_value = jvalue_copy(def_value);
	return validator;
}

static jvalue_ref get_default(Validator *validator, ValidationState *s)
{
	StringValidator *v = (StringValidator *) validator;
	return v->def_value;
}

static Validator* set_max_length_generic(Validator *v, size_t max_length)
{
	return set_max_length(&string_validator_new()->base, max_length);
}

static Validator* set_min_length_generic(Validator *v, size_t min_length)
{
	return set_min_length(&string_validator_new()->base, min_length);
}

static Validator* set_pattern_generic(Validator *v, Pattern *pattern)
{
	return set_pattern(&string_validator_new()->base, pattern);
}

static Validator* set_default_generic(Validator *v, jvalue_ref def_value)
{
	return set_default(&string_validator_new()->base, def_value);
}

static void dump_enter(char const *key, Validator *v, void *ctxt)
{
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	fprintf((FILE *) ctxt, "(s)");
}

static bool equals(Validator *v, Validator *other)
{
	StringValidator *s = (StringValidator *) v;
	StringValidator *s2 = (StringValidator *) other;

	if (s->max_length == s2->max_length &&
	    s->min_length == s2->min_length &&
	    g_strcmp0(s->expected_value, s2->expected_value) == 0)
		return true;

	return false;
}

static ValidatorVtable generic_string_vtable =
{
	.check = check_generic,
	.set_string_max_length = set_max_length_generic,
	.set_string_min_length = set_min_length_generic,
	.set_string_pattern = set_pattern_generic,
	.set_default = set_default_generic,
	.dump_enter = dump_enter,
};

static ValidatorVtable string_vtable =
{
	.check = _check,
	.equals = equals,
	.ref = ref,
	.unref = unref,
	.set_string_max_length = set_max_length,
	.set_string_min_length = set_min_length,
	.set_string_pattern = set_pattern,
	.set_default = set_default,
	.get_default = get_default,
	.dump_enter = dump_enter,
};

StringValidator* string_validator_new(void)
{
	StringValidator *self = g_new0(StringValidator, 1);
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
	if (v->pattern)
		g_regex_unref(v->pattern);
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

void string_validator_set_pattern(StringValidator *v, GRegex *pattern)
{
	if (v->pattern)
		g_regex_unref(v->pattern);
	v->pattern = g_regex_ref(pattern);
}

void string_validator_add_expected_value(StringValidator *v, StringSpan *span)
{
	g_free(v->expected_value);
	v->expected_value = g_strndup(span->str, span->str_len);
}

static Validator STRING_VALIDATOR_IMPL =
{
	.vtable = &generic_string_vtable,
};

Validator *STRING_VALIDATOR_GENERIC = &STRING_VALIDATOR_IMPL;

Validator* string_validator_instance(void)
{
	return STRING_VALIDATOR_GENERIC;
}
