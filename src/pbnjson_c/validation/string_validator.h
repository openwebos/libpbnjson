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

typedef struct _StringSpan StringSpan;

typedef struct _StringValidator
{
	Validator base;

	char *expected_value;
	int min_length;
	int max_length;
} StringValidator;

//_Static_assert(offsetof(StringValidator, base) == 0, "Addresses of StringValidator and StringValidator.base should be equal");

StringValidator* string_validator_new();
void string_validator_release(StringValidator *v);

void string_validator_add_min_length_constraint(StringValidator *v, size_t min_length);
void string_validator_add_max_length_constraint(StringValidator *v, size_t max_length);
bool string_validator_add_expected_value(StringValidator *v, StringSpan *span);

#ifdef __cplusplus
}
#endif
