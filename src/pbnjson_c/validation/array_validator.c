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

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
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
		validation_state_pop_validator(s);
		return res;
	}

	// FIXME: Does YAJL verify basic layout?
	//if (e->type == EV_OBJ_KEY)
	//{
	//	validation_state_pop_validator(s);
	//	return false;
	//}

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

static void _release(Validator *validator)
{
	ArrayValidator *v = (ArrayValidator *) validator;
	array_validator_release(v);
}

static void _set_items(Validator *v, ArrayItems *items)
{
	ArrayValidator *a = (ArrayValidator *) v;
	if (a->items)
		array_items_unref(a->items);
	a->items = array_items_ref(items);
}

static void _set_additional_items(Validator *v, Validator *additional)
{
	ArrayValidator *a = (ArrayValidator *) v;
	if (a->additional_items)
		validator_unref(a->additional_items);
	a->additional_items = validator_ref(additional);
}

static void _set_max_items(Validator *v, size_t max)
{
	ArrayValidator *a = (ArrayValidator *) v;
	array_validator_set_max_items(a, max);
}

static void _set_min_items(Validator *v, size_t min)
{
	ArrayValidator *a = (ArrayValidator *) v;
	array_validator_set_min_items(a, min);
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

static void _dump_enter(char const *key, Validator *v, void *ctxt)
{
	if (key)
		fprintf((FILE *) ctxt, "%s:", key);
	fprintf((FILE *) ctxt, "[");
}

static void _dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	fprintf((FILE *) ctxt, "]");
}

ValidatorVtable array_vtable =
{
	.check = _check,
	.init_state = _init_state,
	.cleanup_state = _cleanup_state,
	.release = _release,
	.visit = _visit,
	.set_array_items = _set_items,
	.set_array_additional_items = _set_additional_items,
	.set_array_max_items = _set_max_items,
	.set_array_min_items = _set_min_items,
	.dump_enter = _dump_enter,
	.dump_exit = _dump_exit,
};

ArrayValidator* array_validator_new(void)
{
	ArrayValidator *self = g_new0(ArrayValidator, 1);
	if (!self)
		return NULL;
	self->max_items = -1;
	self->min_items = -1;
	validator_init(&self->base, &array_vtable);
	self->additional_items = validator_ref(GENERIC_VALIDATOR);
	return self;
}

void array_validator_release(ArrayValidator *v)
{
	if (v->items)
		array_items_unref(v->items);

	if (v->additional_items)
		validator_unref(v->additional_items);

	g_free(v);
}

bool array_validator_set_max_items(ArrayValidator *a, size_t max)
{
	if (a->min_items == -1)
		a->min_items = 0;
	a->max_items = max;
	return true;
}

bool array_validator_set_min_items(ArrayValidator *a, size_t min)
{
	if (a->max_items == -1)
		a->max_items = EMPTY_LENGTH;
	a->min_items = min;
	return true;
}
