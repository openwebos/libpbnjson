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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _StringSpan StringSpan;

/** @brief Reference validator class */
typedef struct _Reference
{
	Validator base;        /**< @brief Base class */
	unsigned ref_count;    /**< @brief Reference count */
	jvalue_ref def_value;  /**< @brief Default value attached to this validator */

	char *target;          /**< @brief Original parsed value like "other.json#/definitions/a" */

	char const *document;  /**< @brief Document part of the reference "other.json", owned by UriResolver */
	char *fragment;        /**< @brief Fragment part of the reference "#/definitions/a" */
	Validator *validator;  /**< @brief Resolved validator, owned by UriResolver */
} Reference;

/** @brief Constructor */
Reference *reference_new(void);

/** @brief Increment reference counter. */
Reference *reference_ref(Reference *r);

/** @brief Decrement reference counter. Once it drops to zero, the object is destructed. */
void reference_unref(Reference *r);

/** @brief Remember target from the parser. */
bool reference_set_target(Reference *r, StringSpan *target);

#ifdef __cplusplus
}
#endif
