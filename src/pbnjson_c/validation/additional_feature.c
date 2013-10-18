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

#include "additional_feature.h"
#include "validator.h"
#include <glib.h>
#include <assert.h>

static void _release(Feature *f)
{
	AdditionalFeature *a = (AdditionalFeature *) f;
	validator_unref(a->validator);
	g_free(a);
}

static Validator* apply(Feature *f, Validator *v)
{
	AdditionalFeature *a = (AdditionalFeature *) f;
	assert(a && a->apply_func);
	return a->apply_func(v, a->validator);
}

static FeatureVtable additional_feature_vtable =
{
	.release = _release,
	.apply = apply,
};

AdditionalFeature* additional_feature_new(AdditionalFeatureFunc apply_func)
{
	AdditionalFeature *a = g_new0(AdditionalFeature, 1);
	feature_init(&a->base, &additional_feature_vtable);
	a->apply_func = apply_func;
	return a;
}

AdditionalFeature* additional_feature_ref(AdditionalFeature *a)
{
	return (AdditionalFeature *) feature_ref(&a->base);
}

void additional_feature_unref(AdditionalFeature *a)
{
	feature_unref(&a->base);
}

void additional_feature_set_validator(AdditionalFeature *a, Validator *v)
{
	validator_unref(a->validator);
	a->validator = v;
}
