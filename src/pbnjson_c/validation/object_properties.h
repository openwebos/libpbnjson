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
typedef struct _ValidationState ValidationState;

/** @brief Object properties class */
typedef struct _ObjectProperties
{
	Feature base;      /**< @brief Base class */
	GHashTable *keys;  /**< @brief Hash map key -> validator for object properties */
} ObjectProperties;


/** @brief Constructor */
ObjectProperties* object_properties_new(void);

/** @brief Increment reference counter. */
ObjectProperties* object_properties_ref(ObjectProperties *o);

/** @brief Decrement reference counter. Once it drops to zero, the object is destructed. */
void object_properties_unref(ObjectProperties *o);

/** @brief Remember a NULL-terminated key with corresponding validator.
 *
 * The key is copied, but the validator is moved.
 *
 * @param[in] o This object
 * @param[in] key Key in {"properties": {"key": {...}}}
 * @param[in] v Validator for subschema in {"properties": {"key": {...}}}
 * @return true if succeeded, false if OOM
 */
bool object_properties_add_key(ObjectProperties *o, char const *key, Validator *v);

/** @brief Remember a key with corresponding validator.
 *
 * The key is copied, but the validator is moved.
 *
 * @param[in] o This object
 * @param[in] key Key in {"properties": {"key": {...}}}
 * @param[in] key_len Length of the key
 * @param[in] v Validator for subschema in {"properties": {"key": {...}}}
 * @return true if succeeded, false if OOM
 */
bool object_properties_add_key_n(ObjectProperties *o, char const *key, size_t key_len, Validator *v);

/** @brief Calculate the count of properties. */
size_t object_properties_length(ObjectProperties *o);

/** @brief Find the validator for a given NULL-terminated key. */
Validator* object_properties_lookup(ObjectProperties *o, char const *key);

/** @brief Find the validator for a given key. */
Validator* object_properties_lookup_n(ObjectProperties *o, char const *key, size_t key_len);

/** @brief Visit contained validators. */
void object_properties_visit(ObjectProperties *o,
                             VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                             void *ctxt);

/** @brief Create hash table of the properties with default values.
 *
 * @return Hash table property key -> jvalue
 */
GHashTable *object_properties_gather_default(ObjectProperties *o, ValidationState *s);

#ifdef __cplusplus
}
#endif
