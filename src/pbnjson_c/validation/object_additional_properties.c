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

#include "object_additional_properties.h"
#include "validator.h"
#include <glib.h>

static void _release(Feature *f)
{
	ObjectAdditionalProperties *p = (ObjectAdditionalProperties *) f;
	validator_unref(p->validator);
	g_free(p);
}

static bool _apply(Feature *f, Validator *v)
{
	ObjectAdditionalProperties *p = (ObjectAdditionalProperties *) f;
	if (!p || !v)
		return false;
	validator_set_object_additional_properties(v, p->validator);
	return true;
}

static FeatureVtable object_additional_properties_vtable =
{
	.release = _release,
	.apply = _apply,
};

ObjectAdditionalProperties* object_additional_properties_new(void)
{
	ObjectAdditionalProperties *p = g_new0(ObjectAdditionalProperties, 1);
	if (!p)
		return p;
	feature_init(&p->base, &object_additional_properties_vtable);
	return p;
}

ObjectAdditionalProperties* object_additional_properties_ref(ObjectAdditionalProperties *p)
{
	return (ObjectAdditionalProperties *) feature_ref(&p->base);
}

void object_additional_properties_unref(ObjectAdditionalProperties *p)
{
	feature_unref(&p->base);
}

void object_additional_properties_set_validator(ObjectAdditionalProperties *p, Validator *v)
{
	validator_unref(p->validator);
	p->validator = v;
}
