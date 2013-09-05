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
typedef struct _Feature Feature;
typedef struct _Definitions Definitions;


typedef struct _SchemaParsing
{
	// We'll keep SchemaParsing instead of validators during parsing.
	Validator base;

	GSList *features;
	Validator *type_validator;
	char *id;
	Definitions *definitions;
	GSList *validator_combinators;
	Validator *extends;
} SchemaParsing;


SchemaParsing* schema_parsing_new(void);
SchemaParsing* schema_parsing_ref(SchemaParsing *s);
void schema_parsing_unref(SchemaParsing *s);

bool schema_parsing_add_feature(SchemaParsing *s, Feature *feature);
void schema_parsing_set_validator(SchemaParsing *s, Validator *v);
bool schema_parsing_set_id(SchemaParsing *s, StringSpan id);
void schema_parsing_set_definitions(SchemaParsing *s, Definitions *d);
bool schema_parsing_add_combinator(SchemaParsing *s, Validator *v);
bool schema_parsing_set_extends(SchemaParsing *s, Validator *extends);

#ifdef __cplusplus
}
#endif
