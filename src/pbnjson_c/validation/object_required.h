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

typedef struct _ObjectRequired
{
	Feature base;
	// Hash table char * -> char const * is used as a set
	GHashTable *keys;
} ObjectRequired;


ObjectRequired* object_required_new(void);
ObjectRequired* object_required_ref(ObjectRequired *o);
void object_required_unref(ObjectRequired *o);

guint object_required_size(ObjectRequired *o);

bool object_required_add_key(ObjectRequired *o, char const *key);
bool object_required_add_key_n(ObjectRequired *o, char const *key, size_t key_len);

char const *object_required_lookup_key(ObjectRequired *o, char const *key);
char const *object_required_lookup_key_n(ObjectRequired *o, char const *key, size_t key_len);

#ifdef __cplusplus
}
#endif
