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

typedef struct _CombinedValidator
{
	Validator base;
	GSList *validators;
	bool (*check_all)(ValidationEvent const *e, ValidationState *s, void *ctxt, bool *all_finished);
} CombinedValidator;

//_Static_assert(offsetof(ArrayValidator, base) == 0, "");

CombinedValidator* combined_validator_new();
void combined_validator_release(CombinedValidator *v);

CombinedValidator* all_of_validator_new();
CombinedValidator* any_of_validator_new();
CombinedValidator* one_of_validator_new();

void combined_validator_convert_to_all_of(CombinedValidator *v);
void combined_validator_convert_to_any_of(CombinedValidator *v);
void combined_validator_convert_to_one_of(CombinedValidator *v);

void combined_validator_add_value(CombinedValidator *a, Validator *v);

#ifdef __cplusplus
}
#endif
