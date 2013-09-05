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
#include "number.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _StringSpan StringSpan;

typedef struct _NumberValidator
{
	Validator base;
	bool integer;

	bool expected_set;
	Number expected_value;

	bool max_set;
	Number max;
	bool max_exclusive;

	bool min_set;
	Number min;
	bool min_exclusive;
} NumberValidator;

//_Static_assert(offsetof(NumberValidator, base) == 0, "Addresses of NumberValidator and NumberValidator.base should be equal");

NumberValidator* number_validator_new();
NumberValidator* integer_validator_new();
void number_validator_release(NumberValidator *v);

// Methods for unit tests
bool number_validator_add_min_constraint(NumberValidator *n, const char* val);
bool number_validator_add_min_exclusive_constraint(NumberValidator *n, bool exclusive);
bool number_validator_add_max_constraint(NumberValidator *n, const char* val);
bool number_validator_add_max_exclusive_constraint(NumberValidator *n, bool exclusive);

bool number_validator_add_expected_value(NumberValidator *n, StringSpan *span);

#ifdef __cplusplus
}
#endif
