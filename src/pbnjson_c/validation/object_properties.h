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

typedef struct _ObjectProperties
{
	Feature base;
	GHashTable *keys;
} ObjectProperties;


ObjectProperties* object_properties_new(void);
ObjectProperties* object_properties_ref(ObjectProperties *o);
void object_properties_unref(ObjectProperties *o);

bool object_properties_add_key(ObjectProperties *o, char const *key, Validator *v);
bool object_properties_add_key_n(ObjectProperties *o, char const *key, size_t key_len, Validator *v);

size_t object_properties_length(ObjectProperties *o);
Validator* object_properties_lookup(ObjectProperties *o, char const *key);
Validator* object_properties_lookup_n(ObjectProperties *o, char const *key, size_t key_len);
void object_properties_visit(ObjectProperties *o,
                             VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                             void *ctxt);

GHashTable *object_properties_gather_default(ObjectProperties *o, ValidationState *s);

#ifdef __cplusplus
}
#endif
