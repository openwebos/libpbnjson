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

#include "validator.h"
#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _StringSpan StringSpan;
typedef struct _Validator Validator;
typedef struct _UriResolver UriResolver;

typedef struct _Definitions
{
	// We want definitions to be visitable
	Validator base;

	char *name;
	GSList *validators;
} Definitions;

Definitions* definitions_new(void);
void definitions_unref(Definitions *d);

bool definitions_set_name(Definitions *d, StringSpan *name);
bool definitions_add(Definitions *d, StringSpan *name, Validator *v);

#ifdef __cplusplus
}
#endif
