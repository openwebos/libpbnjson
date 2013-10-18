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

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Additional properties or items applicator.
 *
 * @param[in] v Validator to apply to
 * @param[in] additional Validator for "additionalProperties" or "additionalItems"
 * @return Validator with feature applied
 */
typedef Validator* (*AdditionalFeatureFunc)(Validator *v, Validator *additional);

/**
 * AdditionalFeature contains parsed "additionalProperties" or "additionalItems"
 */
typedef struct _AdditionalFeature
{
	Feature base;                        /**< @brief Base class */
	Validator *validator;                /**< @brief Correspondive validator */
	AdditionalFeatureFunc apply_func;    /**< @brief Function to be called to apply the feature */
} AdditionalFeature;

/** @brief Constructor */
AdditionalFeature* additional_feature_new(AdditionalFeatureFunc apply_func);

/** @brief Increment reference counter */
AdditionalFeature* additional_feature_ref(AdditionalFeature *p);

/** @brief Decrement reference counter. Once it drops to zero, destroy the feature. */
void additional_feature_unref(AdditionalFeature *p);

/** @brief Remember validator in the feature. Move semantics. */
void additional_feature_set_validator(AdditionalFeature *p, Validator *v);

#ifdef __cplusplus
}
#endif
