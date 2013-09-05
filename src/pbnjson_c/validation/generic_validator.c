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

#include "generic_validator.h"
#include "validation_state.h"
#include "validation_event.h"
#include <glib.h>

static void _release(Validator *v)
{
	g_free(v);
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	// The generic validator has tricky logic. It should validate a single JSON value.
	// If the value is of simple type, implementaion is trivial. But array and
	// object must be tracked carefully, because they can be nested.
	// We rely on the parser to check the order of nested objects and arrays
	// boundaries.

	int depth = GPOINTER_TO_INT(validation_state_get_context(s));
	if (!depth)
	{
		// If the event isn't a beginning of object or array, consume it,
		// and we're done.
		if (e->type != EV_OBJ_START && e->type != EV_ARR_START)
		{
			validation_state_pop_validator(s);
			return true;
		}
	}

	switch (e->type)
	{
	case EV_OBJ_START:
	case EV_ARR_START:
		validation_state_set_context(s, GINT_TO_POINTER(++depth));
		return true;
	case EV_OBJ_END:
	case EV_ARR_END:
		validation_state_set_context(s, GINT_TO_POINTER(--depth));
		break;
	default:
		return true;
	}
	if (!depth)
	{
		validation_state_pop_validator(s);
		return true;
	}
	return true;
}

static bool _init_state(Validator *v, ValidationState *s)
{
	validation_state_push_context(s, (void *)0);
	return true;
}

static void _cleanup_state(Validator *v, ValidationState *s)
{
	validation_state_pop_context(s);
}

static void _dump_enter(char const *key, Validator *v, void *ctxt)
{
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	fprintf((FILE *) ctxt, "(*)");
}

static ValidatorVtable generic_vtable =
{
	.release = _release,
	.check = _check,
	.init_state = _init_state,
	.cleanup_state = _cleanup_state,
	.dump_enter = _dump_enter,
};

Validator *generic_validator_new(void)
{
	Validator *v = g_new0(Validator, 1);
	if (!v)
		return NULL;
	validator_init(v, &generic_vtable);
	return v;
}

Validator *generic_validator_ref(Validator *v)
{
	return validator_ref(v);
}

void generic_validator_unref(Validator *v)
{
	return validator_unref(v);
}

static Validator GENERIC_VALIDATOR_IMPL =
{
	.ref_count = 1,
	.vtable = &generic_vtable,
};

Validator *GENERIC_VALIDATOR = &GENERIC_VALIDATOR_IMPL;
