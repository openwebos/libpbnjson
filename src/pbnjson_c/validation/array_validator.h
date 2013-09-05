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
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ArrayItems ArrayItems;

typedef struct _ArrayValidator
{
	Validator base;
	ArrayItems *items;
	Validator *additional_items;
	int max_items;
	int min_items;
} ArrayValidator;

//_Static_assert(offsetof(ArrayValidator, base) == 0, "");

ArrayValidator* array_validator_new();
void array_validator_release(ArrayValidator *v);

bool array_validator_set_max_items(ArrayValidator *v, size_t max);
bool array_validator_set_min_items(ArrayValidator *v, size_t min);

#ifdef __cplusplus
}
#endif
