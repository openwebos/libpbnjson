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

typedef enum _ValidatorType
{
	V_NULL = 0,
	V_NUM,
	V_BOOL,
	V_STR,
	V_ARR,
	V_OBJ,
	V_ANY,   // TODO: Remove support for "any"

	V_TYPES_NUM,
} ValidatorType;

typedef struct _CombinedTypesValidator
{
	Validator base;
	Validator* types[V_TYPES_NUM];
} CombinedTypesValidator;

//_Static_assert(offsetof(ArrayValidator, base) == 0, "");

CombinedTypesValidator* combined_types_validator_new();
void combined_types_validator_release(CombinedTypesValidator *v);

void combined_types_validator_set_type(CombinedTypesValidator *c, const char *type_str, size_t len);
void combined_types_validator_fill_all_types(CombinedTypesValidator *c);

#ifdef __cplusplus
}
#endif
