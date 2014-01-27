// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
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
#include <glib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * String pattern for {"pattern": "..."}
 */
typedef struct _Pattern
{
	Feature base;    /**< @brief Base class */
	GRegex *regex;   /**< @brief Regular expression handler */
} Pattern;

/** @brief Constructor */
Pattern* pattern_new(void);

/** @brief Assign regular expression to pattern feature */
bool pattern_set_regex(Pattern *p, char const *regex);

/** @brief Assign regular expression to pattern feature */
bool pattern_set_regex_n(Pattern *p, char const *regex, size_t regex_len);


#ifdef __cplusplus
}
#endif
