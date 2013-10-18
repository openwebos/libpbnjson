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

/** @brief String validator class for {"type": "string"} */
typedef struct _StringValidator
{
	Validator base;        /**< @brief Base class */
	unsigned ref_count;    /**< @brief Reference count */
	jvalue_ref def_value;  /**< @brief Default value attached to this validator */

	char *expected_value;  /**< @brief Expected value if not NULL (for enums) */
	int min_length;        /**< @brief Minimal string length from {"minLength": ...} */
	int max_length;        /**< @brief Maximal string length from {"maxLength": ...} */
} StringValidator;

//_Static_assert(offsetof(StringValidator, base) == 0, "Addresses of StringValidator and StringValidator.base should be equal");

/** @brief Generic string validator. Checks only value type. */
Validator* string_validator_instance();

/** @brief Constructor */
StringValidator* string_validator_new();

/** @brief Destructor */
void string_validator_release(StringValidator *v);

/** @brief Remember minimal length */
void string_validator_add_min_length_constraint(StringValidator *v, size_t min_length);

/** @brief Remember maximal length */
void string_validator_add_max_length_constraint(StringValidator *v, size_t max_length);

/** @brief Remember expected value (for enums) */
void string_validator_add_expected_value(StringValidator *v, StringSpan *span);

#ifdef __cplusplus
}
#endif
