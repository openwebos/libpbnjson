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

#include "boolean_feature.h"
#include "validator.h"
#include <assert.h>
#include <glib.h>

static void _release(Feature *f)
{
	BooleanFeature *n = (BooleanFeature *) f;
	g_free(n);
}

static bool _apply(Feature *f, Validator *v)
{
	BooleanFeature *n = (BooleanFeature *) f;
	assert(n->apply_func);
	n->apply_func(v, n->value);
	return true;
}

static FeatureVtable boolean_feature_vtable =
{
	.release = _release,
	.apply = _apply,
};

BooleanFeature* boolean_feature_new(BooleanFeatureFunc apply_func)
{
	BooleanFeature *n = g_new0(BooleanFeature, 1);
	feature_init(&n->base, &boolean_feature_vtable);
	n->apply_func = apply_func;
	return n;
}

BooleanFeature* boolean_feature_ref(BooleanFeature *n)
{
	return (BooleanFeature *) feature_ref(&n->base);
}

void boolean_feature_unref(BooleanFeature *n)
{
	feature_unref(&n->base);
}

void boolean_feature_set_value(BooleanFeature *n, bool value)
{
	assert(n);
	n->value = value;
}
