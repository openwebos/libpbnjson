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

#include "count_feature.h"
#include "number.h"
#include "validator.h"
#include <assert.h>

static void _release(Feature *f)
{
	CountFeature *n = (CountFeature *) f;
	g_free(n);
}

static bool _apply(Feature *f, Validator *v)
{
	CountFeature *n = (CountFeature *) f;
	assert(n->apply_func);
	n->apply_func(v, n->count);
	return true;
}

static FeatureVtable count_feature_vtable =
{
	.release = _release,
	.apply = _apply,
};

CountFeature* count_feature_new(CountFeatureFunc apply_func)
{
	CountFeature *n = g_new0(CountFeature, 1);
	feature_init(&n->base, &count_feature_vtable);
	n->apply_func = apply_func;
	return n;
}

CountFeature* count_feature_ref(CountFeature *n)
{
	return (CountFeature *) feature_ref(&n->base);
}

void count_feature_unref(CountFeature *n)
{
	feature_unref(&n->base);
}

bool count_feature_set_value(CountFeature *n, const char *val, size_t val_len)
{
	assert(n);

	Number num;
	number_init(&num);
	if (number_set_n(&num, val, val_len) ||
	    !number_is_integer(&num) ||
	    !number_fits_long(&num))
	{
		number_clear(&num);
		return false;
	}
	n->count = number_get_long(&num);
	number_clear(&num);
	return n->count >= 0;
}
