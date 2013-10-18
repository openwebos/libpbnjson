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


/** @brief Number validator class for {"type": "number"} */
typedef struct _NumberValidator
{
	Validator base;          /**< @brief Base class */
	unsigned ref_count;      /**< @brief Reference count */
	jvalue_ref def_value;    /**< @brief Default value attached to this validator */

	bool integer;            /**< @brief Should a valid instance be integer? */

	bool expected_set;       /**< @brief true if a specific value is expected (for enum) */
	Number expected_value;   /**< @brief Expected value */

	bool max_set;            /**< @brief Is maximal value set? */
	Number max;              /**< @brief The maximal value to expect */
	bool max_exclusive;      /**< @brief Is the inequality strict (val < max)? */

	bool min_set;            /**< @brief Is the minimal value set? */
	Number min;              /**< @brief The minimal value to expect */
	bool min_exclusive;      /**< @brief Is the inequality strict (val > min)? */
} NumberValidator;

//_Static_assert(offsetof(NumberValidator, base) == 0, "Addresses of NumberValidator and NumberValidator.base should be equal");

/** @brief Generic number validator. Checks only value type */
Validator* number_validator_instance();

/** @brief Constructor. */
NumberValidator* number_validator_new();

/** @brief Generic integer validator. Checks only value type */
Validator* integer_validator_instance();

/** @brief Constructor for integer validator. */
NumberValidator* integer_validator_new();

/** @brief Destructor. */
void number_validator_release(NumberValidator *v);

// Methods for unit tests
bool number_validator_add_min_constraint(NumberValidator *n, const char* val);
void number_validator_add_min_exclusive_constraint(NumberValidator *n, bool exclusive);
bool number_validator_add_max_constraint(NumberValidator *n, const char* val);
void number_validator_add_max_exclusive_constraint(NumberValidator *n, bool exclusive);

/** @brief Set an exact expected value (for enum validation) */
bool number_validator_add_expected_value(NumberValidator *n, StringSpan *span);

#ifdef __cplusplus
}
#endif
