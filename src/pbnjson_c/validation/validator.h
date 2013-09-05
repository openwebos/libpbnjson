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

#include "validator_fwd.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ValidationEvent ValidationEvent;
typedef struct _ValidationState ValidationState;
typedef struct _ObjectProperties ObjectProperties;
typedef struct _ObjectAdditionalProperties ObjectAdditionalProperties;
typedef struct _ObjectRequired ObjectRequired;
typedef struct _ArrayItems ArrayItems;
typedef struct _UriResolver UriResolver;
typedef struct _Number Number;
typedef struct jvalue* jvalue_ref;


typedef struct _ValidatorVtable
{
	// Destructor
	void (*release)(Validator *v);

	// Validation functions
	bool (*check)(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt);
	bool (*init_state)(Validator *v, ValidationState *s);
	void (*cleanup_state)(Validator *v, ValidationState *s);
	void (*reactivate)(Validator *v, ValidationState *s);

	// Parsing functions
	// Descend to children and execute the function (not recursively)
	void (*visit)(Validator *v,
	              VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
	              void *ctxt);

	// Move parsed features into validators
	void (*apply_features)(char const *key, Validator *v, void *ctxt);
	// Combine type validator with other validator containers (allOf, anyOf, etc.)
	void (*combine_validators)(char const *key, Validator *v, void *ctxt, Validator **new_v);
	// Post process the tree: substitute SchemaParsing with Validator
	void (*finalize_parse)(char const *key, Validator *v, void *ctxt, Validator **new_v);
	// Mark URI scope for every SchemaParsing
	void (*collect_uri_enter)(char const *key, Validator *v, void *ctxt);
	void (*collect_uri_exit)(char const *key, Validator *v, void *ctxt, Validator **new_v);

	// Dump validator tree for debugging purposes
	void (*dump_enter)(char const *key, Validator *v, void *ctxt);
	void (*dump_exit)(char const *key, Validator *v, void *ctxt, Validator **new_v);

	void (*set_object_properties)(Validator *v, ObjectProperties *p);
	void (*set_object_additional_properties)(Validator *v, Validator *additional);
	void (*set_object_required)(Validator *v, ObjectRequired *p);
	void (*set_object_max_properties)(Validator *v, size_t max);
	void (*set_object_min_properties)(Validator *v, size_t min);
	void (*set_array_items)(Validator *v, ArrayItems *a);
	void (*set_array_additional_items)(Validator *v, Validator *additional);
	void (*set_array_max_items)(Validator *v, size_t maxItems);
	void (*set_array_min_items)(Validator *v, size_t minItems);
	void (*set_number_maximum)(Validator *v, Number *n);
	void (*set_number_maximum_exclusive)(Validator *v, bool exclusive);
	void (*set_number_minimum)(Validator *v, Number *n);
	void (*set_number_minimum_exclusive)(Validator *v, bool exclusive);
	void (*set_string_max_length)(Validator *v, size_t maxLength);
	void (*set_string_min_length)(Validator *v, size_t minLength);
	void (*set_default)(Validator *v, jvalue_ref def_value);
	jvalue_ref (*get_default)(Validator *v, ValidationState *s);
} ValidatorVtable;

typedef struct _Validator
{
	unsigned ref_count;
	ValidatorVtable *vtable;
	jvalue_ref def_value;
} Validator;

void validator_init(Validator *v, ValidatorVtable *vtable);
Validator* validator_ref(Validator *v);
void validator_unref(Validator *v);

// Virtual
bool validator_check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt);
bool validator_init_state(Validator *v, ValidationState *s);
void validator_cleanup_state(Validator *v, ValidationState *s);
void validator_reactivate(Validator *v, ValidationState *s);

void validator_visit(Validator *v,
                     VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                     void *ctxt);
void VISITOR_ENTER_VOID(char const *key, Validator *v, void *ctxt);
void VISITOR_EXIT_VOID(char const *key, Validator *v, void *ctxt, Validator **new_v);

void _validator_apply_features(char const *key, Validator *v, void *ctxt);
void validator_apply_features(Validator *v);

void _validator_combine(char const *key, Validator *v, void *ctxt, Validator **new_v);
void validator_combine(Validator *v);

void _validator_finalize_parse(char const *key, Validator *v, void *ctxt, Validator **new_v);
Validator* validator_finalize_parse(Validator *v);

void _validator_collect_uri_enter(char const *key, Validator *v, void *ctxt);
void _validator_collect_uri_exit(char const *key, Validator *v, void *ctxt, Validator **new_v);
void validator_collect_uri(Validator *v, char const *document, UriResolver *u);

void _validator_dump_enter(char const *key, Validator *v, void *ctxt);
void _validator_dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v);
void validator_dump(Validator *v, FILE *f);

void validator_set_object_properties(Validator *v, ObjectProperties *p);
void validator_set_object_additional_properties(Validator *v, Validator *additional);
void validator_set_object_required(Validator *v, ObjectRequired *p);
void validator_set_object_max_properties(Validator *v, size_t max);
void validator_set_object_min_properties(Validator *v, size_t min);
void validator_set_array_items(Validator *v, ArrayItems *a);
void validator_set_array_additional_items(Validator *v, Validator *additional);
void validator_set_array_max_items(Validator *v, size_t maxItems);
void validator_set_array_min_items(Validator *v, size_t minItems);
void validator_set_number_maximum(Validator *v, Number *n);
void validator_set_number_maximum_exclusive(Validator *v, bool exclusive);
void validator_set_number_minimum(Validator *v, Number *n);
void validator_set_number_minimum_exclusive(Validator *v, bool exclusive);
void validator_set_string_max_length(Validator *v, size_t maxLength);
void validator_set_string_min_length(Validator *v, size_t minLength);
void validator_set_default(Validator *v, jvalue_ref def_value);
jvalue_ref validator_get_default(Validator *v, ValidationState *s);

#ifdef __cplusplus
}
#endif
