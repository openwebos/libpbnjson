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

#include "array_validator.h"
#include "validation_state.h"
#include "validation_event.h"
#include "generic_validator.h"
#include "array_items.h"
#include "validation_api.h"
#include <jobject.h>
#include <glib.h>
#include <string.h>

#define EMPTY_LENGTH -1

typedef struct _MyContext
{
	bool has_started;     // Has an array been opened with "["?
	size_t items_count;
	GList *cur_validator; // pointer to current validator
} MyContext;

static Validator* _get_current_validator(ArrayValidator *varr, MyContext *ctxt)
{
	if (!varr->items)
		return GENERIC_VALIDATOR;

	ArrayItems *items = varr->items;
	if (items->generic_validator)
		return items->generic_validator;

	GList *cur = ctxt->cur_validator;
	if (!cur)
		return varr->additional_items;

	Validator *v = cur->data;
	ctxt->cur_validator = g_list_next(cur);

	return v;
}

static bool check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	ArrayValidator *varr = (ArrayValidator *) v;
	MyContext *my_ctxt = (MyContext *) validation_state_get_context(s);
	if (!my_ctxt->has_started)
	{
		if (e->type != EV_ARR_START)
		{
			validation_state_notify_error(s, VEC_NOT_ARRAY, c);
			validation_state_pop_validator(s);
			return false;
		}
		my_ctxt->has_started = true;
		my_ctxt->cur_validator = varr->items ? varr->items->validators : NULL;
		return true;
	}

	if (e->type == EV_ARR_END)
	{
		bool res = true;
		if (varr->min_items != -1 && my_ctxt->items_count < varr->min_items)
		{
			validation_state_notify_error(s, VEC_ARRAY_TOO_SHORT, c);
			res = false;
		}
		else if (varr->unique_items && s->notify && s->notify->has_array_duplicates &&
		         s->notify->has_array_duplicates(s, c))
		{
			validation_state_notify_error(s, VEC_ARRAY_HAS_DUPLICATES, c);
			res = false;
		}
		validation_state_pop_validator(s);
		return res;
	}

	++my_ctxt->items_count;

	if (varr->max_items != -1 && my_ctxt->items_count > varr->max_items)
	{
		validation_state_notify_error(s, VEC_ARRAY_TOO_LONG, c);
		validation_state_pop_validator(s);
		return false;
	}

	Validator* vcur = _get_current_validator(varr, my_ctxt);
	if (vcur)
	{
		validation_state_push_validator(s, vcur);
		bool valid = validation_check(e, s, c);
		if (!valid)
			validation_state_pop_validator(s);
		return valid;
	}

	validation_state_notify_error(s, VEC_ARRAY_TOO_LONG, c);
	validation_state_pop_validator(s);
	return false;
}

static bool check_generic(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	// depth is used to track when current and all nested arrays are closed
	// layout checks done by YAJL
	int depth = GPOINTER_TO_INT(validation_state_get_context(s));
	switch (e->type)
	{
	case EV_ARR_START:
		validation_state_set_context(s, GINT_TO_POINTER(++depth));
		return true;
	case EV_ARR_END:
		validation_state_set_context(s, GINT_TO_POINTER(--depth));
		if (!depth)
			// all arrays are closed. Last one was array validated by this object
			// so all validations are passed
			validation_state_pop_validator(s);
		return true;
	default:
		if (!depth)
		{
			// if there is non ARR_START event when depth is zero (a.k.a. basic array is not started yet)
			// than it's not array at all.
			// return error
			validation_state_notify_error(s, VEC_NOT_ARRAY, ctxt);
			validation_state_pop_validator(s);
			return false;
		}

		// if basic array is started (depth is non-zero) we don't care. All is fine
		return true;
	}
}

static bool init_state_generic(Validator *v, ValidationState *s)
{
	validation_state_push_context(s, GINT_TO_POINTER(0));
	return true;
}

static void cleanup_state_generic(Validator *v, ValidationState *s)
{
	validation_state_pop_context(s);
}

static bool _init_state(Validator *v, ValidationState *s)
{
	MyContext *my_ctxt = g_slice_new0(MyContext);
	my_ctxt->has_started = false;
	validation_state_push_context(s, my_ctxt);
	return true;
}

static void _cleanup_state(Validator *v, ValidationState *s)
{
	MyContext *c = validation_state_pop_context(s);
	g_slice_free(MyContext, c);
}

static Validator* ref(Validator *validator)
{
	ArrayValidator *v = (ArrayValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	ArrayValidator *v = (ArrayValidator *) validator;
	if (--v->ref_count)
		return;
	array_validator_release(v);
}

static Validator* set_items(Validator *v, ArrayItems *items)
{
	ArrayValidator *a = (ArrayValidator *) v;
	if (a->items)
		array_items_unref(a->items);
	a->items = array_items_ref(items);
	return v;
}

static Validator* set_additional_items(Validator *v, Validator *additional)
{
	ArrayValidator *a = (ArrayValidator *) v;
	if (a->additional_items)
		validator_unref(a->additional_items);
	a->additional_items = validator_ref(additional);
	return v;
}

static Validator* set_max_items(Validator *v, size_t max)
{
	ArrayValidator *a = (ArrayValidator *) v;
	array_validator_set_max_items(a, max);
	return v;
}

static Validator* set_min_items(Validator *v, size_t min)
{
	ArrayValidator *a = (ArrayValidator *) v;
	array_validator_set_min_items(a, min);
	return v;
}

static Validator* set_unique_items(Validator *v, bool unique)
{
	ArrayValidator *a = (ArrayValidator *) v;
	a->unique_items = unique;
	return v;
}

static Validator* set_default(Validator *v, jvalue_ref def_value)
{
	ArrayValidator *a = (ArrayValidator *) v;
	j_release(&a->def_value);
	a->def_value = jvalue_copy(def_value);
	return v;
}

static jvalue_ref get_default(Validator *v, ValidationState *s)
{
	ArrayValidator *a = (ArrayValidator *) v;
	return a->def_value;
}

static Validator* set_items_generic(Validator *v, ArrayItems *items)
{
	return set_items(&array_validator_new()->base, items);
}

static Validator* set_additional_items_generic(Validator *v, Validator *additional)
{
	return set_additional_items(&array_validator_new()->base, additional);
}

static Validator* set_max_items_generic(Validator *v, size_t max)
{
	return set_max_items(&array_validator_new()->base, max);
}

static Validator* set_min_items_generic(Validator *v, size_t min)
{
	return set_min_items(&array_validator_new()->base, min);
}

static Validator* set_unique_items_generic(Validator *v, bool unique)
{
	return set_unique_items(&array_validator_new()->base, unique);
}

static Validator* set_default_generic(Validator *v, jvalue_ref def_value)
{
	return set_default(&array_validator_new()->base, def_value);
}

static void _visit(Validator *v,
                   VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                   void *ctxt)
{
	ArrayValidator *a = (ArrayValidator *) v;
	if (a->additional_items)
	{
		enter_func(NULL, a->additional_items, ctxt);
		validator_visit(a->additional_items, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, a->additional_items, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(a->additional_items);
			a->additional_items = new_v;
		}
	}
	if (a->items)
		array_items_visit(a->items, enter_func, exit_func, ctxt);
}

static void dump_enter(char const *key, Validator *v, void *ctxt)
{
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	fprintf((FILE *) ctxt, "[");
}

static void dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	fprintf((FILE *) ctxt, "]");
}

static bool equals(Validator *v, Validator *other)
{
	ArrayValidator *a = (ArrayValidator *) v;
	ArrayValidator *a2 = (ArrayValidator *) other;

	if (a->max_items == a2->max_items &&
	    a->min_items == a2->min_items &&
	    a->unique_items == a2->unique_items &&
	    array_items_equals(a->items, a2->items) &&
	    validator_equals(a->additional_items, a2->additional_items))
		return true;

	return false;
}

static ValidatorVtable generic_array_vtable =
{
	.check = check_generic,
	.init_state = init_state_generic,
	.cleanup_state = cleanup_state_generic,
	.set_array_items = set_items_generic,
	.set_array_additional_items = set_additional_items_generic,
	.set_array_max_items = set_max_items_generic,
	.set_array_min_items = set_min_items_generic,
	.set_array_unique_items = set_unique_items_generic,
	.set_default = set_default_generic,
	.dump_enter = dump_enter,
	.dump_exit = dump_exit,
};

ValidatorVtable array_vtable =
{
	.check = check,
	.equals = equals,
	.init_state = _init_state,
	.cleanup_state = _cleanup_state,
	.ref = ref,
	.unref = unref,
	.visit = _visit,
	.set_array_items = set_items,
	.set_array_additional_items = set_additional_items,
	.set_array_max_items = set_max_items,
	.set_array_min_items = set_min_items,
	.set_array_unique_items = set_unique_items,
	.set_default = set_default,
	.get_default = get_default,
	.dump_enter = dump_enter,
	.dump_exit = dump_exit,
};

ArrayValidator* array_validator_new(void)
{
	ArrayValidator *self = g_new0(ArrayValidator, 1);
	self->ref_count = 1;
	self->max_items = -1;
	self->min_items = -1;
	self->unique_items = false;
	validator_init(&self->base, &array_vtable);
	self->additional_items = GENERIC_VALIDATOR;
	return self;
}

void array_validator_release(ArrayValidator *v)
{
	if (v->items)
		array_items_unref(v->items);

	if (v->additional_items)
		validator_unref(v->additional_items);

	j_release(&v->def_value);
	g_free(v);
}

void array_validator_set_max_items(ArrayValidator *a, size_t max)
{
	if (a->min_items == -1)
		a->min_items = 0;
	a->max_items = max;
}

void array_validator_set_min_items(ArrayValidator *a, size_t min)
{
	if (a->max_items == -1)
		a->max_items = EMPTY_LENGTH;
	a->min_items = min;
}

static Validator ARRAY_VALIDATOR_IMPL =
{
	.vtable = &generic_array_vtable,
};

Validator *ARRAY_VALIDATOR_GENERIC = &ARRAY_VALIDATOR_IMPL;

Validator* array_validator_instance(void)
{
	return ARRAY_VALIDATOR_GENERIC;
}
