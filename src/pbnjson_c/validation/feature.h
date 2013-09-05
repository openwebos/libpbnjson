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

typedef struct _FeatureVtable
{
	bool (*apply)(Feature *f, Validator *v);
	void (*release)(Feature *f);
} FeatureVtable;

typedef struct _Feature
{
	int ref_count;
	FeatureVtable *vtable;
} Feature;

void feature_init(Feature *f, FeatureVtable *vtable);
Feature* feature_ref(Feature *f);
void feature_unref(Feature *f);
bool feature_apply(Feature *f, Validator *v);

#ifdef __cplusplus
}
#endif
