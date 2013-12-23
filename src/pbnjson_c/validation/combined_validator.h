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


/** @brief Combinator of validators like in "allOf": [...] */
typedef struct _CombinedValidator
{
	Validator base;          /**< @brief Base class */
	unsigned ref_count;      /**< @brief Reference count */
	jvalue_ref def_value;    /**< @brief Default value attached to this validator */

	GSList *validators;      /**< @brief Validators for subschemas to combine */

	/** @brief Checking functions
	 *
	 * @param[in] e Validation event from YAJL to check.
	 * @param[in] s Validation state
	 * @param[in] ctxt User supplied pointer for notification callbacks
	 * @param[out] all_finished true if validation finished completely
	 * @return false if validation failed
	 */
	bool (*check_all)(ValidationEvent const *e, ValidationState *s, void *ctxt, bool *all_finished);

} CombinedValidator;

//_Static_assert(offsetof(ArrayValidator, base) == 0, "");

/** @brief Constructor */
CombinedValidator* combined_validator_new();

/** @brief Destructor */
void combined_validator_release(CombinedValidator *v);

/** @brief Construct validator for {"allOf": [...]} */
CombinedValidator* all_of_validator_new();

/** @brief Construct validator for {"anyOf": [...]} */
CombinedValidator* any_of_validator_new();

/** @brief Construct validator for {"oneOf": [...]} */
CombinedValidator* one_of_validator_new();

/** @brief Construct validator for {"enum": [...]}
 *
 * NOTE: Basically same as anyOf but will return UNEXPECTED_VALUE error in case of failed validation
 *       and do not support duplicate values
 */
CombinedValidator* enum_validator_new();

/** @brief Let this validator turn into {"allOf": [...]} */
void combined_validator_convert_to_all_of(CombinedValidator *v);

/** @brief Let this validator turn into {"anyOf": [...]} */
void combined_validator_convert_to_any_of(CombinedValidator *v);

/** @brief Let this validator turn into {"oneOf": [...]} */
void combined_validator_convert_to_one_of(CombinedValidator *v);

/** @brief Let this validator turn into {"enum": [...]} */
void combined_validator_convert_to_enum(CombinedValidator *v);

/** @brief Add validator for subschema */
void combined_validator_add_value(CombinedValidator *a, Validator *v);

/** @brief Add validator for enum subschema if accetable */
bool combined_validator_add_enum_value(CombinedValidator *a, Validator *v);

/** @brief Changes error notification of combined validator to notify about each inner error
 *
 * By default combined validator will suppress errors from inner validators.
 * To use general error notification callbacks for all inner errors call this method.
 */
void combined_validator_collect_errors(CombinedValidator *v);

/** @brief Changes error notification of combined validator to suppress all inner error
 *
 * Vice versa of combined_validator_collect_errors() function
 */
void combined_validator_suppress_errors(CombinedValidator *v);

#ifdef __cplusplus
}
#endif
