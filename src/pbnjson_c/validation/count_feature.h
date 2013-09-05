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

typedef void (*CountFeatureFunc)(Validator *v, size_t count);

typedef struct _CountFeature
{
	Feature base;
	long count;
	CountFeatureFunc apply_func;
} CountFeature;


CountFeature* count_feature_new(CountFeatureFunc apply_func);
CountFeature* count_feature_ref(CountFeature *n);
void count_feature_unref(CountFeature *n);

bool count_feature_set_value(CountFeature *n, const char *val, size_t val_len);

#ifdef __cplusplus
}
#endif
