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

typedef struct _ObjectProperties ObjectProperties;
typedef struct _ObjectRequired ObjectRequired;

/**
 * Object validator for {"type": "object"}
 */
typedef struct _ObjectValidator
{
	/** @brief Base class is Validator */
	Validator base;
	/** @brief Reference count */
	unsigned ref_count;
	/** @brief Default value attached to this validator */
	jvalue_ref def_value;

	/** @brief Expected object properties from "properties": {...} */
	ObjectProperties *properties;
	/** @brief Additional properties may accept what wasn't described by "properties" */
	Validator *additional_properties;
	/** @brief List of required property names from "required": [...] */
	ObjectRequired *required;
	/** @brief Maximal count of properties */
	int max_properties;
	/** @brief Minimal count of properties */
	int min_properties;

	/** @brief Count of properties, which have default value.
	 *
	 * Default values are contained by the validators in the properties.
	 * We count them before the main work to decide if properties with
	 * defaults should be tracked at all.
	 */
	int default_properties_count;
} ObjectValidator;

//_Static_assert(offsetof(GenericValidator, base) == 0, "");

/** @brief Generic object validator. Checks only value type */
Validator* object_validator_instance(void);

/** @brief Constructor: allocate and initialize new object validator. */
ObjectValidator* object_validator_new(void);

/** @brief Destroy object validator. */
void object_validator_release(ObjectValidator *v);

/** @brief Set maximal count of properties. */
void object_validator_set_max_properties(ObjectValidator *v, size_t max);

/** @brief Set minimal count of properties. */
void object_validator_set_min_properties(ObjectValidator *v, size_t min);

#ifdef __cplusplus
}
#endif
