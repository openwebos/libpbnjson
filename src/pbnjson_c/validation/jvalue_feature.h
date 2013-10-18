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

typedef struct _Validator Validator;
typedef struct jvalue *jvalue_ref;

/** @brief Feature application function.
 *
 * @param[in] v Validator to apply the feature to
 * @param[in] value JSON representation to apply
 * @return Validator with feture applied
 */
typedef Validator* (*JvalueFeatureFunc)(Validator *v, jvalue_ref value);

/** @brief JSON value feature */
typedef struct _JvalueFeature
{
	Feature base;                 /**< Base class */
	jvalue_ref value;             /**< Value to remember */
	JvalueFeatureFunc apply_func; /**< The function to apply the value */
} JvalueFeature;


/** @brief Constructor */
JvalueFeature *jvalue_feature_new(jvalue_ref value, JvalueFeatureFunc apply_func);

/** @brief Increase reference counter. */
JvalueFeature *jvalue_feature_ref(JvalueFeature *j);

/** @brief Decrease reference counter. Once it drops to zero, destruct the object. */
void jvalue_feature_unref(JvalueFeature *f);

#ifdef __cplusplus
}
#endif
