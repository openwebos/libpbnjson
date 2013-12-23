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

typedef struct _UriResolver UriResolver;

/** @brief Object required class for {"required": [...]} */
typedef struct _ObjectRequired
{
	Feature base;      /**< @brief Base class */
	/** @brief Hash table char * -> char const * is used as a set. */
	GHashTable *keys;
} ObjectRequired;


/** @brief Constructor */
ObjectRequired* object_required_new(void);

/** @brief Increment reference counter. */
ObjectRequired* object_required_ref(ObjectRequired *o);

/** @brief Decrement reference counter. Once it drops to zero, the object is destructed. */
void object_required_unref(ObjectRequired *o);

/** @brief Calculate the count of required properties. */
guint object_required_size(ObjectRequired *o);

/** @brief Remember a NULL-terminated key as required. */
bool object_required_add_key(ObjectRequired *o, char const *key);

/** @brief Remember a key as required. */
bool object_required_add_key_n(ObjectRequired *o, char const *key, size_t key_len);

/** @brief Check if the given NULL-terminated key is among the required.
 *
 * @return Pointer to the original required key value in the set.
 */
char const *object_required_lookup_key(ObjectRequired *o, char const *key);

/** @brief Check if the given key is among the required.
 *
 * @return Pointer to the original required key value in the set.
 */
char const *object_required_lookup_key_n(ObjectRequired *o, char const *key, size_t key_len);

/** @brief Check if two ObjectRequired structures are equal. */
bool object_required_equals(ObjectRequired *o, ObjectRequired *other);

#ifdef __cplusplus
}
#endif
