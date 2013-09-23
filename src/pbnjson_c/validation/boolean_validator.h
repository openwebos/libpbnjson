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


/** @brief JSON boolean validator */
typedef struct _BooleanValidator
{
	Validator base;        /**< Base class */

	bool value_expected;   /**< Is specific value expected? */
	bool value;            /**< The value that the instance is expected to have. */
} BooleanValidator;

/** @brief Constructor */
BooleanValidator *boolean_validator_new(void);

/** @brief Constructor with specific expected value (for enum). */
BooleanValidator *boolean_validator_new_with_value(bool value);

/** @brief Increment reference counter. */
BooleanValidator *boolean_validator_ref(BooleanValidator *b);

/** @brief Decrement reference counter. Once it drops to zero, destruct the object. */
void boolean_validator_unref(BooleanValidator *b);

#ifdef __cplusplus
}
#endif
