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

#include "array_items.h"
#include "validator.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void _validator_release(gpointer data)
{
	validator_unref((Validator *) data);
}

static void _release(Feature *f)
{
	ArrayItems *a = (ArrayItems *) f;
	if (a->generic_validator)
		validator_unref(a->generic_validator);
	g_list_free_full(a->validators, _validator_release);
	g_free(a);
}

static bool _apply(Feature *f, Validator *v)
{
	ArrayItems *a = (ArrayItems *) f;
	validator_set_array_items(v, a);
	return true;
}

static FeatureVtable array_items_vtable =
{
	.release = _release,
	.apply = _apply,
};

ArrayItems* array_items_new(void)
{
	ArrayItems *a = g_new0(ArrayItems, 1);
	feature_init(&a->base, &array_items_vtable);
	return a;
}

ArrayItems* array_items_ref(ArrayItems *a)
{
	return (ArrayItems *) feature_ref(&a->base);
}

void array_items_unref(ArrayItems *a)
{
	feature_unref(&a->base);
}

void array_items_set_generic_item(ArrayItems *a, Validator *v)
{
	// clean up old items before setting general one
	g_list_free_full(a->validators, _validator_release);
	a->validators = NULL;
	if (a->generic_validator)
		validator_unref(a->generic_validator);

	a->generic_validator = v;
}

void array_items_add_item(ArrayItems *a, Validator *v)
{
	if (a->generic_validator)
		validator_unref(a->generic_validator), a->generic_validator = NULL;

	a->validators = g_list_append(a->validators, v);
	++a->validator_count;
}

size_t array_items_items_length(ArrayItems *a)
{
	assert(a->validator_count == g_list_length(a->validators));
	return a->validator_count;
}

void array_items_set_zero_items(ArrayItems *a)
{
	if (a->generic_validator)
		validator_unref(a->generic_validator), a->generic_validator = NULL;

	g_list_free_full(a->validators, _validator_release);
	a->validators = NULL;
	a->validator_count = 0;
}

void array_items_visit(ArrayItems *a,
                       VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                       void *ctxt)
{
	if (a->generic_validator)
	{
		enter_func(NULL, a->generic_validator, ctxt);
		validator_visit(a->generic_validator, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, a->generic_validator, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(a->generic_validator);
			a->generic_validator = new_v;
		}
	}

	GList *it = a->validators;
	while (it)
	{
		Validator *v = (Validator *) it->data;
		enter_func(NULL, v, ctxt);
		validator_visit(v, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, v, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(v);
			it->data = new_v;
		}
		it = g_list_next(it);
	}
}
