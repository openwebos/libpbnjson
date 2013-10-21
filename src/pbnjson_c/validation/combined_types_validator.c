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

#include "combined_types_validator.h"
#include "array_items.h"
#include "object_properties.h"
#include "generic_validator.h"
#include "null_validator.h"
#include "boolean_validator.h"
#include "number_validator.h"
#include "string_validator.h"
#include "object_validator.h"
#include "array_validator.h"
#include "validation_state.h"
#include "validation_event.h"
#include "parser_context.h"
#include "type_parser.h"
#include <jobject.h>
#include <assert.h>

static Validator* _get_current_validator(CombinedTypesValidator *c, ValidationEvent const *e)
{
	switch (e->type)
	{
	case EV_NULL:
		return c->types[V_NULL];
	case EV_NUM:
		return c->types[V_NUM];
	case EV_STR:
		return c->types[V_STR];
	case EV_BOOL:
		return c->types[V_BOOL];
	case EV_ARR_START:
		return c->types[V_ARR];
	case EV_OBJ_START:
		return c->types[V_OBJ];
	default:
		return NULL;
	}
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *c)
{
	CombinedTypesValidator *vcomb = (CombinedTypesValidator *) v;
	Validator *vcur = _get_current_validator(vcomb, e);
	if (!vcur)
	{
		validation_state_notify_error(s, VEC_TYPE_NOT_ALLOWED, c);
		validation_state_pop_validator(s);
		return false;
	}

	validation_state_pop_validator(s);
	validation_state_push_validator(s, vcur);
	return validator_check(vcur, e, s, c);
}

static Validator* ref(Validator *validator)
{
	CombinedTypesValidator *v = (CombinedTypesValidator *) validator;
	++v->ref_count;
	return validator;
}

static void unref(Validator *validator)
{
	CombinedTypesValidator *v = (CombinedTypesValidator *) validator;
	if (--v->ref_count)
		return;
	combined_types_validator_release(v);
}

static void _set_maximum(Validator *v, Number *n)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_NUM])
	{
		NumberValidator *new = number_validator_new();
		if (!new)
			return;
		c->types[V_NUM] = &new->base;
	}
	validator_set_number_maximum(c->types[V_NUM], n);
}

static void _set_maximum_exclusive(Validator *v, bool exclusive)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_NUM])
	{
		NumberValidator *new = number_validator_new();
		if (!new)
			return;
		c->types[V_NUM] = &new->base;
	}
	validator_set_number_maximum_exclusive(c->types[V_NUM], exclusive);
}

static void _set_minimum(Validator *v, Number *n)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_NUM])
	{
		NumberValidator *new = number_validator_new();
		if (!new)
			return;
		c->types[V_NUM] = &new->base;
	}
	validator_set_number_minimum(c->types[V_NUM], n);
}

static void _set_minimum_exclusive(Validator *v, bool exclusive)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_NUM])
	{
		NumberValidator *new = number_validator_new();
		if (!new)
			return;
		c->types[V_NUM] = &new->base;
	}
	validator_set_number_minimum_exclusive(c->types[V_NUM], exclusive);
}

static void _set_max_length(Validator *v, size_t maxLength)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_STR])
	{
		StringValidator *new = string_validator_new();
		if (!new)
			return;
		c->types[V_STR] = &new->base;
	}
	validator_set_string_max_length(c->types[V_STR], maxLength);
}

static void _set_min_length(Validator *v, size_t minLength)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_STR])
	{
		StringValidator *new = string_validator_new();
		if (!new)
			return;
		c->types[V_STR] = &new->base;
	}
	validator_set_string_min_length(c->types[V_STR], minLength);
}

static void _set_items(Validator *v, ArrayItems *items)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_ARR])
	{
		ArrayValidator *new = array_validator_new();
		if (!new)
			return;
		c->types[V_ARR] = &new->base;
	}
	validator_set_array_items(c->types[V_ARR], items);
}

static void _set_additional_items(Validator *v, Validator *additional)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_ARR])
	{
		ArrayValidator *new = array_validator_new();
		if (!new)
			return;
		c->types[V_ARR] = &new->base;
	}
	validator_set_array_additional_items(c->types[V_ARR], additional);
}

static void _set_properties(Validator *v, ObjectProperties *p)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_OBJ])
	{
		ObjectValidator *new = object_validator_new();
		if (!new)
			return;
		c->types[V_OBJ] = &new->base;
	}
	validator_set_object_properties(c->types[V_OBJ], p);
}

static void _set_additional_properties(Validator *v, Validator *additional)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	if (!c->types[V_OBJ])
	{
		ObjectValidator *new = object_validator_new();
		if (!new)
			return;
		c->types[V_OBJ] = &new->base;
	}
	validator_set_object_additional_properties(c->types[V_OBJ], additional);
}

static void set_default(Validator *validator, jvalue_ref def_value)
{
	CombinedTypesValidator *v = (CombinedTypesValidator *) validator;
	j_release(&v->def_value);
	v->def_value = jvalue_copy(def_value);
}

static jvalue_ref get_default(Validator *validator, ValidationState *s)
{
	CombinedTypesValidator *v = (CombinedTypesValidator *) validator;
	return v->def_value;
}

static void _visit(Validator *v,
                   VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                   void *ctxt)
{
	CombinedTypesValidator *c = (CombinedTypesValidator *) v;
	int i = 0;
	for (; i < V_TYPES_NUM; ++i)
	{
		Validator *v = c->types[i];
		if (!v)
			continue;
		enter_func(NULL, v, ctxt);
		validator_visit(v, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, v, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(v);
			c->types[i] = new_v;
		}
	}
}

ValidatorVtable combined_types_vtable =
{
	.check = _check,
	.ref = ref,
	.unref = unref,
	.visit = _visit,
	.set_number_maximum = _set_maximum,
	.set_number_maximum_exclusive = _set_maximum_exclusive,
	.set_number_minimum = _set_minimum,
	.set_number_minimum_exclusive = _set_minimum_exclusive,
	.set_string_max_length = _set_max_length,
	.set_string_min_length = _set_min_length,
	.set_array_items = _set_items,
	.set_array_additional_items = _set_additional_items,
	.set_object_properties = _set_properties,
	.set_object_additional_properties = _set_additional_properties,
	.set_default = set_default,
	.get_default = get_default,
};

CombinedTypesValidator* combined_types_validator_new(void)
{
	CombinedTypesValidator *self = g_new0(CombinedTypesValidator, 1);
	if (!self)
		return NULL;
	self->ref_count = 1;
	validator_init(&self->base, &combined_types_vtable);
	return self;
}

void combined_types_validator_release(CombinedTypesValidator *v)
{
	int i = 0;
	for (; i < V_TYPES_NUM; ++i)
		validator_unref(v->types[i]);

	j_release(&v->def_value);
	g_free(v);
}

void combined_types_validator_set_type(CombinedTypesValidator *c, const char *type_str, size_t len)
{
	StringSpan str = { .str = type_str, .str_len = len };
	ValidatorType type = type_parser_parse_to_type(&str, NULL);
	assert(type < V_TYPES_NUM);

	Validator *old = c->types[type];
	if (old)
		// if there is a number validator skip it (needed to resolve integer and number types simultaneously)
		if (type == V_NUM && !((NumberValidator *) old)->integer)
			return;
		validator_unref(old);

	c->types[type] = type_parser_parse_simple(&str, NULL);
}

void combined_types_validator_fill_all_types(CombinedTypesValidator *c)
{
	int i = 0;
	for (; i < V_TYPES_NUM; ++i)
	{
		if (!c->types[i])
		{
			switch (i)
			{
			case V_NULL:
				c->types[i] = NULL_VALIDATOR;
				break;
			case V_BOOL:
				c->types[i] = (Validator *) boolean_validator_new();
				break;
			case V_NUM:
				c->types[i] = (Validator *) number_validator_new();
				break;
			case V_STR:
				c->types[i] = (Validator *) string_validator_new();
				break;
			default:
				c->types[i] = GENERIC_VALIDATOR;
			}
		}
	}
}
