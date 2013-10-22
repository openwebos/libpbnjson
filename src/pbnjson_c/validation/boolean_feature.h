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

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Application function for the feature.
 *
 * @param[in] v Validator to apply the feature to.
 * @param[in] value Value to apply.
 */
typedef void (*BooleanFeatureFunc)(Validator *v, bool value);

typedef struct _BooleanFeature
{
	Feature base;
	bool value;
	BooleanFeatureFunc apply_func;
} BooleanFeature;


/** @brief Constructor */
BooleanFeature* boolean_feature_new(BooleanFeatureFunc apply_func);

/** @brief Increment reference counter. */
BooleanFeature* boolean_feature_ref(BooleanFeature *n);

/** @brief Decrement reference counter. Once it drops to zero, the object is destructed. */
void boolean_feature_unref(BooleanFeature *n);

/** @brief Remember the boolean value */
void boolean_feature_set_value(BooleanFeature *n, bool value);

#ifdef __cplusplus
}
#endif
