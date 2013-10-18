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

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Validator Validator;
typedef struct _Feature Feature;

/**
 * Table of virtual functions of Feature
 */
typedef struct _FeatureVtable
{
	/** @brief Apply this feature to a validator.
	 *
	 * @param[in] f This feature.
	 * @param[in] v Validator to apply the feature to.
	 * @return Validator with feature applied
	 */
	Validator* (*apply)(Feature *f, Validator *v);

	/** @brief Destructor: destroy the contained data. */
	void (*release)(Feature *f);
} FeatureVtable;

/**
 * Schema feature like "properties", "items", "minLength" etc.
 */
typedef struct _Feature
{
	/** @brief Reference count. */
	int ref_count;
	/** @brief Pointer to the table of virtual functions. */
	FeatureVtable *vtable;
} Feature;


/** @brief Initialize the structure.
 *
 * Sets reference count to 1, remembers the table of virtual functions.
 */
void feature_init(Feature *f, FeatureVtable *vtable);

/** @brief Increate reference count. */
Feature* feature_ref(Feature *f);

/** @brief Decrease reference count. Once it drops to zero, the destructor is engaged. */
void feature_unref(Feature *f);

/** @brief Apply this feature to the validator */
Validator* feature_apply(Feature *f, Validator *v);

#ifdef __cplusplus
}
#endif
