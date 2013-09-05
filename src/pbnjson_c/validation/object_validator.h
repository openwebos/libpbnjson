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

#include "validator.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ObjectProperties ObjectProperties;
typedef struct _ObjectRequired ObjectRequired;

typedef struct _ObjectValidator
{
	Validator base;
	ObjectProperties *properties;
	Validator *additional_properties;
	ObjectRequired *required;
	int max_properties;
	int min_properties;

	int default_properties_count;
} ObjectValidator;

//_Static_assert(offsetof(GenericValidator, base) == 0, "");

ObjectValidator* object_validator_new(void);
void object_validator_release(ObjectValidator *v);

bool object_validator_set_max_properties(ObjectValidator *v, size_t max);
bool object_validator_set_min_properties(ObjectValidator *v, size_t min);

#ifdef __cplusplus
}
#endif
