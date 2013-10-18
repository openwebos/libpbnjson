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

/** @brief Null type validator. */
typedef struct _NullValidator
{
	Validator base;        /**< @brief Base class */
	unsigned ref_count;    /**< @brief Reference count */
	jvalue_ref def_value;  /**< @brief Default value attached to this validator */
} NullValidator;

/** @brief Static instance of a null validator. */
extern Validator *NULL_VALIDATOR;

/** @brief Getter of static instance */
Validator *null_validator_instance(void);

/** @brief Constructor: allocate and initialize a new null validator. */
NullValidator *null_validator_new(void);

#ifdef __cplusplus
}
#endif
