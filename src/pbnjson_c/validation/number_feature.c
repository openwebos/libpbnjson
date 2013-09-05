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

#include "number_feature.h"
#include "validator.h"
#include <glib.h>
#include <assert.h>


static void _release(Feature *f)
{
	NumberFeature *n = (NumberFeature *) f;
	number_clear(&n->value);
	g_free(n);
}

static bool _apply(Feature *f, Validator *v)
{
	NumberFeature *n = (NumberFeature *) f;
	assert(n->apply_func);
	n->apply_func(v, &n->value);
	return true;
}

static FeatureVtable number_feature_vtable =
{
	.release = _release,
	.apply = _apply,
};

NumberFeature* number_feature_new(char const *str, size_t len,
                                  NumberFeatureFunc apply_func)
{
	NumberFeature *n = g_new0(NumberFeature, 1);
	if (!n)
		return NULL;
	n->apply_func = apply_func;
	number_init(&n->value);
	if (number_set_n(&n->value, str, len))
	{
		g_free(n);
		return NULL;
	}
	feature_init(&n->base, &number_feature_vtable);
	return n;
}

NumberFeature* number_feature_ref(NumberFeature *n)
{
	return (NumberFeature *) feature_ref(&n->base);
}

void number_feature_unref(NumberFeature *n)
{
	feature_unref(&n->base);
}
