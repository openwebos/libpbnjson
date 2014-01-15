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

#include "number_validator.h"
#include "number.h"
#include "validation_state.h"
#include "validation_event.h"
#include "parser_context.h"
#include <jobject.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


static bool _check_conditions(NumberValidator *v, Number const *n,
                              ValidationState *s, void *ctxt)
{
	if (v->integer && !number_is_integer(n))
	{
		validation_state_notify_error(s, VEC_NOT_INTEGER_NUMBER, ctxt);
		return false;
	}

	if (v->expected_set && 0 != number_compare(n, &v->expected_value))
	{
		validation_state_notify_error(s, VEC_UNEXPECTED_VALUE, ctxt);
		return false;
	}

	if (v->min_set)
	{
		int cmp = number_compare(n, &v->min);
		if (cmp == -1 || (cmp == 0 && v->min_exclusive))
		{
			validation_state_notify_error(s, VEC_NUMBER_TOO_SMALL, ctxt);
			return false;
		}
	}

	if (v->max_set)
	{
		int cmp = number_compare(n, &v->max);
		if (cmp == 1 || (cmp == 0 && v->max_exclusive))
		{
			validation_state_notify_error(s, VEC_NUMBER_TOO_BIG, ctxt);
			return false;
		}
	}

	if (v->multiple_of_set)
	{
		Number div;
		number_init(&div);
		number_div(n, &v->multiple_of, &div);
		bool res = number_is_integer(&div);
		number_clear(&div);
		if (!res)
		{
			validation_state_notify_error(s, VEC_NUMBER_NOT_MULTIPLE_OF, ctxt);
			return false;
		}
	}

	return true;
}

static bool check_generic(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	validation_state_pop_validator(s);
	if (e->type != EV_NUM)
	{
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		return false;
	}
	return true;
}

static bool check_integer_conditions(Number *n, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	if (number_set_n(n, e->value.string.ptr, e->value.string.len))
	{
		// TODO: Number format error
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		return false;
	}

	if (!number_is_integer(n))
	{
		validation_state_notify_error(s, VEC_NOT_INTEGER_NUMBER, ctxt);
		return false;
	}

	return true;
}

static bool check_integer_generic(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	if (e->type != EV_NUM)
	{
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	Number n;
	number_init(&n);

	bool res = check_integer_conditions(&n, e, s, ctxt);
	number_clear(&n);
	validation_state_pop_validator(s);
	return res;
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	if (e->type != EV_NUM)
	{
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	Number n;
	number_init(&n);
	if (number_set_n(&n, e->value.string.ptr, e->value.string.len))
	{
		number_clear(&n);
		// TODO: Number format error
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	bool res = _check_conditions((NumberValidator *) v, &n, s, ctxt);

	number_clear(&n);
	validation_state_pop_validator(s);
	return res;
}

static Validator* ref(Validator *validator)
{
	NumberValidator *v = (NumberValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	NumberValidator *v = (NumberValidator *) validator;
	if (--v->ref_count)
		return;
	number_validator_release(v);
}

static Validator* set_maximum(Validator *v, Number *num)
{
	NumberValidator *n = (NumberValidator *) v;
	n->max_set = true;
	number_init(&n->max);
	number_copy(&n->max, num);
	return v;
}

static Validator* set_maximum_exclusive(Validator *v, bool exclusive)
{
	NumberValidator *n = (NumberValidator *) v;
	number_validator_add_max_exclusive_constraint(n, exclusive);
	return v;
}

static Validator* set_minimum(Validator *v, Number *num)
{
	NumberValidator *n = (NumberValidator *) v;
	n->min_set = true;
	number_init(&n->min);
	number_copy(&n->min, num);
	return v;
}

static Validator* set_minimum_exclusive(Validator *v, bool exclusive)
{
	NumberValidator *n = (NumberValidator *) v;
	number_validator_add_min_exclusive_constraint(n, exclusive);
	return v;
}

static Validator* set_multiple_of(Validator *v, Number *num)
{
	NumberValidator *n = (NumberValidator *) v;
	n->multiple_of_set = true;
	number_init(&n->multiple_of);
	number_copy(&n->multiple_of, num);
	return v;
}

static Validator* set_default(Validator *validator, jvalue_ref def_value)
{
	NumberValidator *v = (NumberValidator *) validator;
	j_release(&v->def_value);
	v->def_value = jvalue_copy(def_value);
	return validator;
}

static jvalue_ref get_default(Validator *validator, ValidationState *s)
{
	NumberValidator *v = (NumberValidator *) validator;
	return v->def_value;
}

static Validator* set_maximum_generic(Validator *v, Number *num)
{
	return set_maximum(&number_validator_new()->base, num);
}

static Validator* set_minimum_generic(Validator *v, Number *num)
{
	return set_minimum(&number_validator_new()->base, num);
}

static Validator* set_maximum_exclusive_generic(Validator *v, bool exclusive)
{
	return set_maximum_exclusive(&number_validator_new()->base, exclusive);
}

static Validator* set_minimum_exclusive_generic(Validator *v, bool exclusive)
{
	return set_minimum_exclusive(&number_validator_new()->base, exclusive);
}

static Validator* set_multiple_of_generic(Validator *v, Number *num)
{
	return set_multiple_of(&number_validator_new()->base, num);
}

static Validator* set_default_generic(Validator *v, jvalue_ref def_value)
{
	return set_default(&number_validator_new()->base, def_value);
}

static Validator* set_maximum_integer_generic(Validator *v, Number *num)
{
	return set_maximum(&integer_validator_new()->base, num);
}

static Validator* set_minimum_integer_generic(Validator *v, Number *num)
{
	return set_minimum(&integer_validator_new()->base, num);
}

static Validator* set_maximum_exclusive_integer_generic(Validator *v, bool exclusive)
{
	return set_maximum_exclusive(&integer_validator_new()->base, exclusive);
}

static Validator* set_minimum_exclusive_integer_generic(Validator *v, bool exclusive)
{
	return set_minimum_exclusive(&integer_validator_new()->base, exclusive);
}

static Validator* set_multiple_of_integer_generic(Validator *v, Number *num)
{
	return set_multiple_of(&integer_validator_new()->base, num);
}

static Validator* set_default_integer_generic(Validator *v, jvalue_ref def_value)
{
	return set_default(&integer_validator_new()->base, def_value);
}

static bool equals(Validator *v, Validator *other)
{
	NumberValidator *n = (NumberValidator *) v;
	NumberValidator *n2 = (NumberValidator *) other;

	if (n->integer == n2->integer &&
	    n->expected_set == n2->expected_set &&
	    number_compare(&n->expected_value, &n2->expected_value) == 0 &&
	    n->max_set == n2->max_set &&
	    number_compare(&n->max, &n2->max) == 0 &&
	    n->max_exclusive == n2->max_exclusive &&
	    n->min_set == n2->min_set &&
	    number_compare(&n->min, &n2->min) == 0 &&
	    n->min_exclusive == n2->min_exclusive)
		return true;

	return false;
}

static ValidatorVtable generic_number_vtable =
{
	.check = check_generic,
	.set_number_maximum = set_maximum_generic,
	.set_number_maximum_exclusive = set_maximum_exclusive_generic,
	.set_number_minimum = set_minimum_generic,
	.set_number_minimum_exclusive = set_minimum_exclusive_generic,
	.set_number_multiple_of = set_multiple_of_generic,
	.set_default = set_default_generic,
};

static ValidatorVtable generic_integer_vtable =
{
	.check = check_integer_generic,
	.set_number_maximum = set_maximum_integer_generic,
	.set_number_maximum_exclusive = set_maximum_exclusive_integer_generic,
	.set_number_minimum = set_minimum_integer_generic,
	.set_number_minimum_exclusive = set_minimum_exclusive_integer_generic,
	.set_number_multiple_of = set_multiple_of_integer_generic,
	.set_default = set_default_integer_generic,
};

static ValidatorVtable number_vtable =
{
	.ref = ref,
	.unref = unref,
	.check = _check,
	.equals = equals,
	.set_number_maximum = set_maximum,
	.set_number_maximum_exclusive = set_maximum_exclusive,
	.set_number_minimum = set_minimum,
	.set_number_minimum_exclusive = set_minimum_exclusive,
	.set_number_multiple_of = set_multiple_of,
	.set_default = set_default,
	.get_default = get_default,
};

NumberValidator* number_validator_new(void)
{
	NumberValidator *self = g_new0(NumberValidator, 1);
	self->ref_count = 1;
	validator_init(&self->base, &number_vtable);
	return self;
}

NumberValidator* integer_validator_new(void)
{
	NumberValidator *v = number_validator_new();
	v->integer = true;
	return v;
}

void number_validator_release(NumberValidator *v)
{
	if (v->expected_set)
		number_clear(&v->expected_value);
	if (v->min_set)
		number_clear(&v->min);
	if (v->max_set)
		number_clear(&v->max);
	if (v->multiple_of_set)
		number_clear(&v->multiple_of);
	j_release(&v->def_value);
	g_free(v);
}

bool number_validator_add_min_constraint(NumberValidator *n, const char* val)
{
	Number num;
	number_init(&num);
	if (number_set(&num, val))
	{
		number_clear(&num);
		return false;
	}
	set_minimum(&n->base, &num);
	number_clear(&num);
	return true;
}

void number_validator_add_min_exclusive_constraint(NumberValidator *n, bool exclusive)
{
	n->min_exclusive = exclusive;
}

bool number_validator_add_max_constraint(NumberValidator *n, const char* val)
{
	Number num;
	number_init(&num);
	if (number_set(&num, val))
	{
		number_clear(&num);
		return false;
	}
	set_maximum(&n->base, &num);
	number_clear(&num);
	return true;
}

void number_validator_add_max_exclusive_constraint(NumberValidator *n, bool exclusive)
{
	n->max_exclusive = exclusive;
}

bool number_validator_add_expected_value(NumberValidator *n, StringSpan *span)
{
	if (n->expected_set)
		number_clear(&n->expected_value), n->expected_set = false;

	number_init(&n->expected_value);
	if (number_set_n(&n->expected_value, span->str, span->str_len))
	{
		number_clear(&n->expected_value);
		return false;
	}
	n->expected_set = true;
	return true;
}

static Validator NUMBER_VALIDATOR_IMPL =
{
	.vtable = &generic_number_vtable,
};

Validator *NUMBER_VALIDATOR_GENERIC = &NUMBER_VALIDATOR_IMPL;

Validator* number_validator_instance(void)
{
	return NUMBER_VALIDATOR_GENERIC;
}

static Validator INTEGER_VALIDATOR_IMPL =
{
	.vtable = &generic_integer_vtable,
};

Validator *INTEGER_VALIDATOR_GENERIC = &INTEGER_VALIDATOR_IMPL;

Validator* integer_validator_instance(void)
{
	return INTEGER_VALIDATOR_GENERIC;
}
