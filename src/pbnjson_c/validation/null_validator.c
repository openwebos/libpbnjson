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

#include "null_validator.h"
#include "validation_event.h"
#include "validation_state.h"
#include "error_code.h"
#include <jobject.h>

static Validator* ref(Validator *validator)
{
	NullValidator *v = (NullValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	NullValidator *v = (NullValidator *) validator;
	if (--v->ref_count)
		return;
	j_release(&v->def_value);
	g_free(v);
}

static Validator* set_default(Validator *validator, jvalue_ref def_value)
{
	NullValidator *v = (NullValidator *) validator;
	j_release(&v->def_value);
	v->def_value = jvalue_copy(def_value);
	return validator;
}

static jvalue_ref get_default(Validator *validator, ValidationState *s)
{
	NullValidator *v = (NullValidator *) validator;
	return v->def_value;
}

static Validator* set_default_generic(Validator *v, jvalue_ref def_value)
{
	return set_default(&null_validator_new()->base, def_value);
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	if (e->type != EV_NULL)
		validation_state_notify_error(s, VEC_NOT_NULL, c);
	validation_state_pop_validator(s);
	return e->type == EV_NULL;
}

static ValidatorVtable generic_null_vtable =
{
	.check = _check,
	.set_default = set_default_generic,
};

static ValidatorVtable null_vtable =
{
	.ref = ref,
	.unref = unref,
	.check = _check,
	.set_default = set_default,
	.get_default = get_default,
};

static Validator NULL_VALIDATOR_IMPL =
{
	.vtable = &generic_null_vtable
};

Validator *NULL_VALIDATOR = &NULL_VALIDATOR_IMPL;

NullValidator *null_validator_new(void)
{
	NullValidator *v = g_new0(NullValidator, 1);
	v->ref_count = 1;
	validator_init(&v->base, &null_vtable);
	return v;
}

Validator *null_validator_instance(void)
{
	return NULL_VALIDATOR;
}
