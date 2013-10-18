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

#include "boolean_validator.h"
#include "validation_event.h"
#include "validation_state.h"
#include <jobject.h>

static Validator* ref(Validator *validator)
{
	BooleanValidator *v = (BooleanValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	BooleanValidator *v = (BooleanValidator *) validator;
	if (--v->ref_count)
		return;
	j_release(&v->def_value);
	g_free(v);
}

static bool check_generic(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	validation_state_pop_validator(s);
	if (e->type != EV_BOOL)
	{
		validation_state_notify_error(s, VEC_NOT_BOOLEAN, c);
		return false;
	}
	return true;
}

static bool check_true(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	if (!check_generic(v, e, s, c))
		return false;

	if (e->value.boolean)
		return true;

	validation_state_notify_error(s, VEC_UNEXPECTED_VALUE, c);
	return false;
}

static bool check_false(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	if (!check_generic(v, e, s, c))
		return false;

	if (!e->value.boolean)
		return true;

	validation_state_notify_error(s, VEC_UNEXPECTED_VALUE, c);
	return false;
}

static Validator* set_default(Validator *v, jvalue_ref def_value)
{
	BooleanValidator *b = (BooleanValidator *) v;
	j_release(&b->def_value);
	b->def_value = jvalue_copy(def_value);
	return v;
}

static jvalue_ref get_default(Validator *v, ValidationState *s)
{
	BooleanValidator *b = (BooleanValidator *) v;
	return b->def_value;
}

static Validator* set_default_generic(Validator *v, jvalue_ref def_value)
{
	return set_default(&boolean_validator_new()->base, def_value);
}

static ValidatorVtable boolean_vtable =
{
	.ref = ref,
	.unref = unref,
	.check = check_generic,
	.set_default = set_default,
	.get_default = get_default,
};

static ValidatorVtable generic_boolean_vtable =
{
	.check = check_generic,
	.set_default = set_default_generic,
};

static ValidatorVtable true_boolean_vtable =
{
	.check = check_true,
	.set_default = set_default_generic,
};

static ValidatorVtable false_boolean_vtable =
{
	.check = check_false,
	.set_default = set_default_generic,
};

static Validator GENERIC_BOOLEAN_VALIDATOR =
{
	.vtable = &generic_boolean_vtable
};

static Validator TRUE_BOOLEAN_VALIDATOR =
{
	.vtable = &true_boolean_vtable
};

static Validator FALSE_BOOLEAN_VALIDATOR =
{
	.vtable = &false_boolean_vtable
};

Validator *boolean_validator_instance(void)
{
	return &GENERIC_BOOLEAN_VALIDATOR;
}

BooleanValidator *boolean_validator_new(void)
{
	BooleanValidator *b = g_new0(BooleanValidator, 1);
	b->ref_count = 1;
	validator_init(&b->base, &boolean_vtable);
	return b;
}

Validator *boolean_validator_new_with_value(bool value)
{
	return value ? &TRUE_BOOLEAN_VALIDATOR : &FALSE_BOOLEAN_VALIDATOR;
}
