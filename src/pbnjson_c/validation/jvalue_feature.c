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

#include "jvalue_feature.h"
#include "validator.h"
#include <jobject.h>

static void _release(Feature *f)
{
	JvalueFeature *j = (JvalueFeature *) f;
	j_release(&j->value);
	g_free(j);
}

static bool _apply(Feature *f, Validator *v)
{
	JvalueFeature *j = (JvalueFeature *) f;
	assert(j->apply_func);
	j->apply_func(v, j->value);
	return true;
}

static FeatureVtable jvalue_feature_vtable =
{
	.release = _release,
	.apply = _apply,
};

JvalueFeature *jvalue_feature_new(jvalue_ref value, JvalueFeatureFunc apply_func)
{
	JvalueFeature *j = g_new0(JvalueFeature, 1);
	feature_init(&j->base, &jvalue_feature_vtable);
	j->value = value;
	j->apply_func = apply_func;
	return j;
}

JvalueFeature *jvalue_feature_ref(JvalueFeature *j)
{
	return (JvalueFeature *) feature_ref(&j->base);
}

void jvalue_feature_unref(JvalueFeature *j)
{
	feature_unref(&j->base);
}
