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

#pragma once

#include "feature.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ObjectAdditionalProperties
{
	Feature base;
	Validator *validator;
} ObjectAdditionalProperties;

ObjectAdditionalProperties* object_additional_properties_new(void);
ObjectAdditionalProperties* object_additional_properties_ref(ObjectAdditionalProperties *p);
void object_additional_properties_unref(ObjectAdditionalProperties *p);

void object_additional_properties_set_validator(ObjectAdditionalProperties *p, Validator *v);

#ifdef __cplusplus
}
#endif
