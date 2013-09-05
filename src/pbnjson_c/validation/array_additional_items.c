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

#include "array_additional_items.h"
#include "validator.h"
#include <glib.h>

static void _release(Feature *f)
{
	ArrayAdditionalItems *a = (ArrayAdditionalItems *) f;
	validator_unref(a->validator);
	g_free(a);
}

static bool _apply(Feature *f, Validator *v)
{
	ArrayAdditionalItems *a = (ArrayAdditionalItems *) f;
	if (!a || !v)
		return false;
	validator_set_array_additional_items(v, a->validator);
	return true;
}

static FeatureVtable array_additional_items_vtable =
{
	.release = _release,
	.apply = _apply,
};

ArrayAdditionalItems* array_additional_items_new(void)
{
	ArrayAdditionalItems *a = g_new0(ArrayAdditionalItems, 1);
	if (!a)
		return a;
	feature_init(&a->base, &array_additional_items_vtable);
	return a;
}

ArrayAdditionalItems* array_additional_items_ref(ArrayAdditionalItems *a)
{
	return (ArrayAdditionalItems *) feature_ref(&a->base);
}

void array_additional_items_unref(ArrayAdditionalItems *a)
{
	feature_unref(&a->base);
}

void array_additional_items_set_validator(ArrayAdditionalItems *a, Validator *v)
{
	validator_unref(a->validator);
	a->validator = v;
}
