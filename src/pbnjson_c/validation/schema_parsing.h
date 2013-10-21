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


/**
 * Abstract syntax tree node. This structure is used to keep the data,
 * obtained from the JSON schema during parsing.
 */
typedef struct _SchemaParsing
{
	/** @brief Base class of SchemaParsing.
	 *
	 * SchemaParsing forms the same tree as finished validators.
	 * Therefore SchemaParsing should inherit from Validator.
	 */
	Validator base;

	/** @brief Reference count */
	unsigned ref_count;

	/** @brief List of parsed features like "properties", "items", "additionalProperties" etc */
	GSList *features;

	/** @brief Type validator created from the attribute "type" */
	Validator *type_validator;

	/** @brief Attribute "id", which may change URI scope */
	char *id;

	/** @brief Subsection "definitions", which holds referred subschemas */
	Definitions *definitions;

	/** @brief Combining validator, which was obtained from "oneOf", "allOf" and similar */
	GSList *validator_combinators;

	/** @brief Reference to the inherited validator.
	 *
	 * TODO: It's implemented partially, and isn't supported by current specification.
	 * Remove it.
	 */
	Validator *extends;
} SchemaParsing;


/** @brief Constructor.
 *
 * Allocate new SchemaParsing on the heap, and initialize it.
 * Reference counter is 1.
 */
SchemaParsing* schema_parsing_new(void);

/** @brief Increment reference counter. */
SchemaParsing* schema_parsing_ref(SchemaParsing *s);

/** @brief Decrement reference counter. Once it drops to 0, the structure is destructed, and the memory freed. */
void schema_parsing_unref(SchemaParsing *s);

/** @brief Add schema feature to the list.
 *
 * Implements move semantics: feature's reference counter isn't incremented,
 * and will be decremented when the SchemaParsing is destroyed.
 */
bool schema_parsing_add_feature(SchemaParsing *s, Feature *feature);

/** @brief Set validator created from the attribute "type".
 *
 * Implements move semantics: validator's reference counter isn't incremented,
 * and will be decremented when the SchemaParsing is destroyed.
 */
void schema_parsing_set_validator(SchemaParsing *s, Validator *v);

/** @brief Remember "id" of current subschema.
 *
 * New string is allocated, copying the span from the source.
 */
bool schema_parsing_set_id(SchemaParsing *s, StringSpan id);

/** @brief Remember definitions of subschemas under "definitions".
 *
 * Implements move semantics.
 */
void schema_parsing_set_definitions(SchemaParsing *s, Definitions *d);

/** @brief Remember the combinator of validators like "allOf", "anyOf" etc.
 *
 * Move semantics: validator's reference count isn't incremented, but
 * the ownership is taken.
 */
bool schema_parsing_add_combinator(SchemaParsing *s, Validator *v);

bool schema_parsing_set_extends(SchemaParsing *s, Validator *extends);

#ifdef __cplusplus
}
#endif
