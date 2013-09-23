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
#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Application function
 *
 * @param[in] v Validator to apply the feature to
 * @param[in] count Value to apply
 */
typedef void (*CountFeatureFunc)(Validator *v, size_t count);

/** @brief Count feature */
typedef struct _CountFeature
{
	Feature base;                /**< Base class */
	long count;                  /**< Value of the feature */
	CountFeatureFunc apply_func; /**< Applicator function */
} CountFeature;


/** @brief Constructor */
CountFeature* count_feature_new(CountFeatureFunc apply_func);

/** @brief Increase reference counter. */
CountFeature* count_feature_ref(CountFeature *n);

/** @brief Decrease reference counter. Once it drops to zero, destroy the object. */
void count_feature_unref(CountFeature *n);

/** @brief Remember the value of the feature.
 *
 * @param[in] n This feature
 * @param[in] val Pointer to the chunk of text with the value to remember.
 * @param[in] val_len Length of the number to remember
 * @return true if succeeded
 */
bool count_feature_set_value(CountFeature *n, const char *val, size_t val_len);

#ifdef __cplusplus
}
#endif
