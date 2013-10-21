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


/** @brief Expected validator type */
typedef enum _ValidatorType
{
	V_NULL = 0,   /**< @brief JSON null */
	V_NUM,        /**< @brief JSON number */
	V_BOOL,       /**< @brief JSON boolean */
	V_STR,        /**< @brief JSON string */
	V_ARR,        /**< @brief JSON array */
	V_OBJ,        /**< @brief JSON object */
	V_ANY,        /**< @brief Any JSON type. TODO: It's obsolete, remove it. */

	V_TYPES_NUM,  /**< @brief Count of JSON types. */
} ValidatorType;

/** @brief Validator for type combination */
typedef struct _CombinedTypesValidator
{
	Validator base;                  /**< @brief Base class */
	unsigned ref_count;              /**< @brief Reference count */
	jvalue_ref def_value;            /**< @brief Default value attached to this validator */

	Validator* types[V_TYPES_NUM];   /**< @brief Validators for specified types {"type":[...]}. */
} CombinedTypesValidator;

//_Static_assert(offsetof(ArrayValidator, base) == 0, "");


/** @brief Constructor */
CombinedTypesValidator* combined_types_validator_new();

/** @brief Destructor */
void combined_types_validator_release(CombinedTypesValidator *v);

/** @brief Add validator for a specific type. */
void combined_types_validator_set_type(CombinedTypesValidator *c, const char *type_str, size_t len);

/** @brief Add default validators for the rest in the array. */
void combined_types_validator_fill_all_types(CombinedTypesValidator *c);

#ifdef __cplusplus
}
#endif
