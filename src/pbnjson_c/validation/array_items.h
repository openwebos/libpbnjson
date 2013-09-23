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

#include "feature.h"
#include "validator_fwd.h"
#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Array items for {"items": [...]}
 */
typedef struct _ArrayItems
{
	Feature base;                  /**< @brief Base class */
	Validator *generic_validator;  /**< @brief Validator for {"items": {...}} */
	GList *validators;             /**< @brief Validators for specified elements {"items": [...]} */
	size_t validator_count;        /**< @brief Count of specified array elements */
} ArrayItems;


/** @brief Constructor */
ArrayItems* array_items_new(void);

/** @brief Increment reference counter. */
ArrayItems* array_items_ref(ArrayItems *o);

/** @brief Decrement reference counter. Once it drop to zero, destruct the object. */
void array_items_unref(ArrayItems *o);

/** @brief Remember the generic item validator. Move semantics. */
bool array_items_set_generic_item(ArrayItems *a, Validator *v);

/** @brief Equivalent of "items = []" in array schema. */
bool array_items_set_zero_items(ArrayItems *a);

/** @brief Add a specified item to the list. */
bool array_items_add_item(ArrayItems *a, Validator *v);

/** @brief Access the count of specified items. */
size_t array_items_items_length(ArrayItems *a);

/** @brief Visit contained validators. */
void array_items_visit(ArrayItems *a,
                       VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                       void *ctxt);

#ifdef __cplusplus
}
#endif
