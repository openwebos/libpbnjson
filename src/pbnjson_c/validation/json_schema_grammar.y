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

%token_prefix TOKEN_
%name JsonSchemaParser
%start_symbol top_schema
%extra_argument { ParserContext *context }
%token_type { TokenParam }

%include {
#include "generic_validator.h"
#include "parser_context.h"
#include "object_properties.h"
#include "additional_feature.h"
#include "object_required.h"
#include "array_items.h"
#include "count_feature.h"
#include "number_feature.h"
#include "boolean_feature.h"
#include "combined_types_validator.h"
#include "combined_validator.h"
#include "schema_parsing.h"
#include "type_parser.h"
#include "definitions.h"
#include "reference.h"
#include "null_validator.h"
#include "boolean_validator.h"
#include "string_validator.h"
#include "number_validator.h"
#include "array_validator.h"
#include "object_validator.h"
#include "jvalue_feature.h"

#include <jobject.h>
#include <assert.h>


#define PARSE_FAILED_IF(c) \
	if (c) parser_context_set_error(context, "JSON schema parsing failed");
#define PARSE_FAILED PARSE_FAILED_IF(1);
}

%syntax_error {
PARSE_FAILED;
}

%parse_failure {
PARSE_FAILED;
}

%destructor top_schema
{
	// Let the compiler be happy that context is used in the destructor.
	//context->success = context->success;
}

top_schema ::= schema(A).
{
	context->validator = A ? A : validator_ref(GENERIC_VALIDATOR);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Generic schema
%type schema { Validator * }
%destructor schema { validator_unref($$), $$ = NULL; }


schema(A) ::= OBJ_START
              schema_attribute_list(B)
              OBJ_END.
{
	if (B)
		A = &B->base;
	else
	{
		SchemaParsing *s = schema_parsing_new();
		if (s)
			// No default value is assumed, so use static generic.
			schema_parsing_set_validator(s, validator_ref(GENERIC_VALIDATOR));
		else PARSE_FAILED;
		A = &s->base;
	}
}

schema(A) ::= OBJ_START
              KEY_DREF STRING(B)
              dref_ignore
              OBJ_END.
{
	Reference *r = reference_new();
	if (r && reference_set_target(r, &B.string))
		A = &r->base;
	else
	{
		reference_unref(r);
		PARSE_FAILED;
	}
}

// TODO: Check if this should be removed
dref_ignore ::= .
dref_ignore ::= KEY_DESCRIPTION skip.

/////////////////////////////////////////////////////////////////////////////////////////////////
// List of attributes of schema
%type schema_attribute_list { SchemaParsing * }
%destructor schema_attribute_list { schema_parsing_unref($$), $$ = NULL; }

schema_attribute_list(A) ::= . { A = NULL; }

schema_attribute_list(A) ::= schema_attribute_list(B) schema_type(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_set_validator(A, C);
	else PARSE_FAILED;
}

schema_attribute_list(A) ::= schema_attribute_list(B) schema_feature(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_add_feature(A, C);
	else PARSE_FAILED;
}

schema_attribute_list(A) ::= schema_attribute_list(B) schema_id(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_set_id(A, C);
	else PARSE_FAILED;
}

schema_attribute_list(A) ::= schema_attribute_list(B) schema_enum(C).
{
	A = B ? B : schema_parsing_new();
	// TODO: Add separate way in for the enumeration.
	// It should be possible to specify both "type" and "enum".
	if (A)
		schema_parsing_set_validator(A, C);
	else PARSE_FAILED;
}

schema_attribute_list(A) ::= schema_attribute_list(B) schema_default(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_add_feature(A, C);
	else PARSE_FAILED;
}

// Unknown keywords should be skipped without error
schema_attribute_list(A) ::= schema_attribute_list(B) schema_unknown.
{
	A = B;
}

schema_unknown ::= KEY_NOT_KEYWORD(A) skip.
{
	fprintf(stderr, "Unknown keyword: %.*s\n", (int)A.string.str_len, A.string.str);
}

skip ::= OBJ_START skip_properties_list OBJ_END.
skip_properties_list ::= .
skip_properties_list ::= skip_properties_list any_object_key skip.
skip ::= ARR_START skip_items_list ARR_END.
skip_items_list ::= .
skip_items_list ::= skip_items_list skip.
skip ::= BOOLEAN.
skip ::= STRING.
skip ::= NUMBER.
skip ::= NULL.

/////////////////////////////////////////////////////////////////////////////////////////////////
// Schema features
%type schema_feature { Feature * }
%destructor schema_feature { feature_unref($$), $$ = NULL; }

schema_feature(A) ::= KEY_DSCHEMA STRING.         { A = NULL; }
schema_feature(A) ::= KEY_TITLE STRING.           { A = NULL; }
schema_feature(A) ::= KEY_DESCRIPTION STRING.     { A = NULL; }
schema_feature(A) ::= KEY_NAME STRING.            { A = NULL; }


/////////////////////////////////////////////////////////////////////////////////////////////////
// URI scope
%type schema_id { StringSpan }

schema_id(A) ::= KEY_ID STRING(B).
{
	A = B.string;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions
schema_attribute_list(A) ::= schema_attribute_list(B) schema_definitions(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_set_definitions(A, C);
	else
	{
		PARSE_FAILED;
		definitions_unref(C);
	}
}

%type schema_definitions { Definitions * }
%destructor schema_definitions { definitions_unref($$), $$ = NULL; }

schema_definitions(A) ::= KEY_DEFINITIONS(B) OBJ_START definitions_list(C) OBJ_END.
{
	A = C;
	if (A)
		definitions_set_name(C, &B.string);
}

%type definitions_list { Definitions * }
%destructor definitions_list { definitions_unref($$); }

definitions_list(A) ::= .  { A = NULL; }

definitions_list(A) ::= definitions_list(B) any_object_key(K) schema(V).
{
	A = B ? B : definitions_new();
	if (A)
		definitions_add(A, &K.string, V);
	else PARSE_FAILED;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Combinators
schema_attribute_list(A) ::= schema_attribute_list(B) schema_combinator(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_add_combinator(A, C);
	else PARSE_FAILED;
}

%type schema_combinator { Validator * }
%destructor schema_combinator { validator_unref($$), $$ = NULL; }

schema_combinator(A) ::= KEY_ANY_OF any_of_body(B).
{
	A = B;
}

schema_combinator(A) ::= KEY_ONE_OF ARR_START combined_validator(B) ARR_END.
{
	if (B)
		combined_validator_convert_to_one_of(B);
	else parser_context_set_error(context, "oneOf array is empty");
	A = &B->base;
}

schema_combinator(A) ::= KEY_ALL_OF all_of_body(B).
{
	A = B;
}


%type any_of_body { Validator * }
%destructor any_of_body { validator_unref($$), $$ = NULL; }

any_of_body(A) ::= ARR_START combined_validator(B) ARR_END.
{
	if (B)
		combined_validator_convert_to_any_of(B);
	else parser_context_set_error(context, "anyOf array is empty");
	A = &B->base;
}

%type all_of_body { Validator * }
%destructor all_of_body { validator_unref($$), $$ = NULL; }

all_of_body(A) ::= ARR_START combined_validator(B) ARR_END.
{
	if (B)
		combined_validator_convert_to_all_of(B);
	else parser_context_set_error(context, "allOf array is empty");
	A = &B->base;
}

%type combined_validator { CombinedValidator * }
%destructor combined_validator { validator_unref(&$$->base), $$ = NULL; }

combined_validator(A) ::= . { A = NULL; }

combined_validator(A) ::= combined_validator(B) schema(V).
{
	A = B ? B : combined_validator_new();
	if (A)
		combined_validator_add_value(A, V);
	else PARSE_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Extends
// TODO: Remove support for extends after all the client code fixed.

schema_attribute_list(A) ::= schema_attribute_list(B) KEY_EXTENDS schema(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_set_extends(A, C);
	else PARSE_FAILED;
}

schema_attribute_list(A) ::= schema_attribute_list(B) KEY_EXTENDS any_of_body(C).
{
	A = B ? B : schema_parsing_new();
	if (A)
		schema_parsing_set_extends(A, C);
	else PARSE_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Instance type
%type schema_type { Validator * }
%destructor schema_type { validator_unref($$), $$ = NULL; }

schema_type(A) ::= KEY_TYPE STRING(B).
{
	enum TypeParserError error = TPE_OK;
	A = type_parser_parse_simple(&B.string, &error);
	switch (error)
	{
	case TPE_OK: break;
	case TPE_UNKNOWN_TYPE: parser_context_set_error(context, "Invalid type"); break;
	default: PARSE_FAILED;
	}
}

schema_type(A) ::= KEY_TYPE ARR_START type_list(B) ARR_END.
{
	A = B;
}

%type type_list { Validator * }
%destructor type_list { validator_unref($$), $$ = NULL; }

type_list(A) ::= STRING(B).
{
	enum TypeParserError error = TPE_OK;
	type_parser_parse_to_type(&B.string, &error);
	switch (error)
	{
	case TPE_OK: break;
	case TPE_UNKNOWN_TYPE: parser_context_set_error(context, "Invalid type"); break;
	default: PARSE_FAILED;
	}
	A = (Validator *) combined_types_validator_new();
	if (A)
		combined_types_validator_set_type((CombinedTypesValidator *) A, B.string.str, B.string.str_len);
	else PARSE_FAILED;
}

type_list(A) ::= type_list(B) STRING(C).
{
	enum TypeParserError error = TPE_OK;
	type_parser_parse_to_type(&C.string, &error);
	switch (error)
	{
	case TPE_OK: break;
	case TPE_UNKNOWN_TYPE: parser_context_set_error(context, "Invalid type"); break;
	default: PARSE_FAILED;
	}
	A = B;
	combined_types_validator_set_type((CombinedTypesValidator *) A, C.string.str, C.string.str_len);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Object properties
schema_feature(A) ::= KEY_PROPERTIES OBJ_START properties(B) OBJ_END.
{
	A = B ? &B->base : NULL;
}

%type properties { ObjectProperties * }
%destructor properties {
    if ($$) object_properties_unref($$), $$ = NULL;
}

properties(A) ::= .
{ A = NULL; }

properties(A) ::= properties(B)
                  any_object_key(K) schema(V).
{
	A = B ? B : object_properties_new();
	if (A)
		object_properties_add_key_n(A, K.string.str, K.string.str_len, V);
	else PARSE_FAILED;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Capture any object key, even keyword.
// Keep this line for automatic check: KEY_NOT_KEYWORD

%type any_object_key { TokenParam }

any_object_key(A) ::= KEY_ADDITIONAL_ITEMS(B). { A = B; }
any_object_key(A) ::= KEY_ADDITIONAL_PROPERTIES(B). { A = B; }
any_object_key(A) ::= KEY_ALL_OF(B). { A = B; }
any_object_key(A) ::= KEY_ANY_OF(B). { A = B; }
any_object_key(A) ::= KEY_DEFAULT(B). { A = B; }
any_object_key(A) ::= KEY_DEFINITIONS(B). { A = B; }
any_object_key(A) ::= KEY_DESCRIPTION(B). { A = B; }
any_object_key(A) ::= KEY_DREF(B). { A = B; }
any_object_key(A) ::= KEY_DSCHEMA(B). { A = B; }
any_object_key(A) ::= KEY_ENUM(B). { A = B; }
any_object_key(A) ::= KEY_EXCLUSIVE_MAXIMUM(B). { A = B; }
any_object_key(A) ::= KEY_EXCLUSIVE_MINIMUM(B). { A = B; }
any_object_key(A) ::= KEY_EXTENDS(B). { A = B; }
any_object_key(A) ::= KEY_ID(B). { A = B; }
any_object_key(A) ::= KEY_ITEMS(B). { A = B; }
any_object_key(A) ::= KEY_MAXIMUM(B). { A = B; }
any_object_key(A) ::= KEY_MAX_ITEMS(B). { A = B; }
any_object_key(A) ::= KEY_MAX_LENGTH(B). { A = B; }
any_object_key(A) ::= KEY_MAX_PROPERTIES(B). { A = B; }
any_object_key(A) ::= KEY_MINIMUM(B). { A = B; }
any_object_key(A) ::= KEY_MIN_ITEMS(B). { A = B; }
any_object_key(A) ::= KEY_MIN_LENGTH(B). { A = B; }
any_object_key(A) ::= KEY_MIN_PROPERTIES(B). { A = B; }
any_object_key(A) ::= KEY_NAME(B). { A = B; }
any_object_key(A) ::= KEY_NOT_KEYWORD(B). { A = B; }
any_object_key(A) ::= KEY_ONE_OF(B). { A = B; }
any_object_key(A) ::= KEY_PROPERTIES(B). { A = B; }
any_object_key(A) ::= KEY_REQUIRED(B). { A = B; }
any_object_key(A) ::= KEY_TITLE(B). { A = B; }
any_object_key(A) ::= KEY_TYPE(B). { A = B; }


/////////////////////////////////////////////////////////////////////////////////////////////////
// Object additionalProperties
schema_feature(A) ::= KEY_ADDITIONAL_PROPERTIES schema(V).
{
	AdditionalFeature *f = additional_feature_new(validator_set_object_additional_properties);
	if (f)
		additional_feature_set_validator(f, V);
	else PARSE_FAILED;
	A = &f->base;
}

schema_feature(A) ::= KEY_ADDITIONAL_PROPERTIES BOOLEAN(B).
{
	AdditionalFeature *f = additional_feature_new(validator_set_object_additional_properties);
	if (f)
		additional_feature_set_validator(f, B.boolean
		                                 ? validator_ref(GENERIC_VALIDATOR)
		                                 : NULL);
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Object required
schema_feature(A) ::= KEY_REQUIRED ARR_START required(B) ARR_END.
{
	// TODO: Warn or issue an error if empty array is collected.
	// We allow its emptiness because of the legacy code.
	A = &B->base;
}

%type required { ObjectRequired * }
%destructor required {
	if ($$) object_required_unref($$), $$ = NULL;
}

required(A) ::= .
{
	A = NULL;
}

required(A) ::= required(B) STRING(K).
{
	A = B ? B : object_required_new();
	if (A)
		object_required_add_key_n(A, K.string.str, K.string.str_len);
	else PARSE_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Object maxProperties
schema_feature(A) ::= KEY_MAX_PROPERTIES NUMBER(N).
{
	CountFeature *f = count_feature_new(validator_set_object_max_properties);
	if (f)
	{
		if (!count_feature_set_value(f, N.string.str, N.string.str_len))
			parser_context_set_error(context, "Invalid maxProperties format");
	}
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Object minProperties
schema_feature(A) ::= KEY_MIN_PROPERTIES NUMBER(N).
{
	CountFeature *f = count_feature_new(validator_set_object_min_properties);
	if (f)
	{
		if (!count_feature_set_value(f, N.string.str, N.string.str_len))
			parser_context_set_error(context, "Invalid minProperties format");
	}
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Array items
schema_feature(A) ::= KEY_ITEMS schema(V).
{
	ArrayItems *a = array_items_new();
	if (a)
		array_items_set_generic_item(a, V);
	else PARSE_FAILED;
	A = &a->base;
}

schema_feature(A) ::= KEY_ITEMS ARR_START items(B) ARR_END.
{
	if (B)
		A = &B->base;
	else
	{
		ArrayItems *a = array_items_new();
		if (a)
			array_items_set_zero_items(a);
		else PARSE_FAILED;
		A = &a->base;
	}
}

%type items { ArrayItems * }
%destructor items {
	if ($$) array_items_unref($$), $$ = NULL;
}

items(A) ::= .
{ A = NULL; }

items(A) ::= items(B) schema(V).
{
	A = B ? B : array_items_new();
	if (A)
		array_items_add_item(A, V);
	else PARSE_FAILED;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Array additionalItems
schema_feature(A) ::= KEY_ADDITIONAL_ITEMS schema(V).
{
	AdditionalFeature *f = additional_feature_new(validator_set_array_additional_items);
	if (f)
		additional_feature_set_validator(f, V);
	else PARSE_FAILED;
	A = &f->base;
}

schema_feature(A) ::= KEY_ADDITIONAL_ITEMS BOOLEAN(B).
{
	AdditionalFeature *f = additional_feature_new(validator_set_array_additional_items);
	if (f)
		additional_feature_set_validator(f, B.boolean
		                                    ? validator_ref(GENERIC_VALIDATOR)
		                                    : NULL);
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Array maxItems
schema_feature(A) ::= KEY_MAX_ITEMS NUMBER(N).
{
	CountFeature *f = count_feature_new(validator_set_array_max_items);
	if (f)
	{
		if (!count_feature_set_value(f, N.string.str, N.string.str_len))
			parser_context_set_error(context, "Invalid maxItems format");
	}
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Array minItems
schema_feature(A) ::= KEY_MIN_ITEMS NUMBER(N).
{
	CountFeature *f = count_feature_new(validator_set_array_min_items);
	if (f)
	{
		if (!count_feature_set_value(f, N.string.str, N.string.str_len))
			parser_context_set_error(context, "Invalid minItems format");
	}
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Number maximum
schema_feature(A) ::= KEY_MAXIMUM NUMBER(N).
{
	NumberFeature *f = number_feature_new(N.string.str, N.string.str_len,
	                                      validator_set_number_maximum);
	if (!f) PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Number exclusiveMaximum
schema_feature(A) ::= KEY_EXCLUSIVE_MAXIMUM BOOLEAN(B).
{
	BooleanFeature *f = boolean_feature_new(validator_set_number_maximum_exclusive);
	if (f)
		boolean_feature_set_value(f, B.boolean);
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Number minimum
schema_feature(A) ::= KEY_MINIMUM NUMBER(N).
{
	NumberFeature *f = number_feature_new(N.string.str, N.string.str_len,
	                                      validator_set_number_minimum);
	if (!f) PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Number exclusiveMinimum
schema_feature(A) ::= KEY_EXCLUSIVE_MINIMUM BOOLEAN(B).
{
	BooleanFeature *f = boolean_feature_new(validator_set_number_minimum_exclusive);
	if (f)
		boolean_feature_set_value(f, B.boolean);
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// String maxLength
schema_feature(A) ::= KEY_MAX_LENGTH NUMBER(N).
{
	CountFeature *f = count_feature_new(validator_set_string_max_length);
	if (f)
	{
		if (!count_feature_set_value(f, N.string.str, N.string.str_len))
			parser_context_set_error(context, "Invalid maxLength format");
	}
	else PARSE_FAILED;
	A = &f->base;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// String minLength
schema_feature(A) ::= KEY_MIN_LENGTH NUMBER(N).
{
	CountFeature *f = count_feature_new(validator_set_string_min_length);
	if (f)
	{
		if (!count_feature_set_value(f, N.string.str, N.string.str_len))
			parser_context_set_error(context, "Invalid minLength format");
	}
	else PARSE_FAILED;
	A = &f->base;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Enum

%type schema_enum { Validator * }
%destructor schema_enum { validator_unref($$), $$ = NULL; }

schema_enum(A) ::= KEY_ENUM ARR_START enum_list(B) ARR_END.
{
	A = &B->base;
}

%type enum_list { CombinedValidator * }
%destructor enum_list { combined_validator_release($$); }

enum_list(A) ::= value_validator(V).
{
	A = one_of_validator_new();
	if (A)
		combined_validator_add_value(A, V);
	else PARSE_FAILED;
}

enum_list(A) ::= enum_list(B) value_validator(V).
{
	A = B;
	combined_validator_add_value(A, V);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Specific value

%type value_validator { Validator * }
%destructor value_validator { validator_unref($$); }

value_validator(A) ::= NULL.
{
	A = null_validator_new();
	if (!A) PARSE_FAILED;
}

value_validator(A) ::= BOOLEAN(V).
{
	BooleanValidator *b = boolean_validator_new_with_value(V.boolean);
	if (!b) PARSE_FAILED;
	A = &b->base;
}

value_validator(A) ::= STRING(V).
{
	StringValidator *s = string_validator_new();
	if (s)
	{
		if (!string_validator_add_expected_value(s, &V.string))
			PARSE_FAILED;
	}
	else PARSE_FAILED;
	A = &s->base;
}

value_validator(A) ::= NUMBER(V).
{
	NumberValidator *n = number_validator_new();
	if (n)
	{
		if (!number_validator_add_expected_value(n, &V.string))
			PARSE_FAILED;
	}
	else PARSE_FAILED;
	A = &n->base;
}

value_validator(A) ::= ARR_START value_validator_items(B) ARR_END.
{
	ArrayValidator *a = array_validator_new();
	A = &a->base;
	if (A)
	{
		size_t count = array_items_items_length(B);
		validator_set_array_max_items(A, count);
		validator_set_array_min_items(A, count);
		validator_set_array_items(A, B);
	}
	else PARSE_FAILED;
	array_items_unref(B);
}

%type value_validator_items { ArrayItems * }
%destructor value_validator_items { array_items_unref($$); }

value_validator_items(A) ::= .
{
	A = array_items_new();
	if (!A) PARSE_FAILED;
}

value_validator_items(A) ::= value_validator_items(B) value_validator(V).
{
	A = B;
	if (!array_items_add_item(A, V)) PARSE_FAILED;
}

value_validator(A) ::= OBJ_START value_validator_properties(B) OBJ_END.
{
	ObjectValidator *o = object_validator_new();
	A = &o->base;
	if (A)
	{
		size_t count = object_properties_length(B);
		validator_set_object_max_properties(A, count);
		validator_set_object_min_properties(A, count);
		validator_set_object_properties(A, B);
		validator_set_object_additional_properties(A, NULL);
	}
	else PARSE_FAILED;
	object_properties_unref(B);
}

%type value_validator_properties { ObjectProperties * }
%destructor value_validator_properties { object_properties_unref($$); }

value_validator_properties(A) ::= .
{
	A = object_properties_new();
	if (!A) PARSE_FAILED;
}

value_validator_properties(A) ::= value_validator_properties(B) any_object_key(K) value_validator(V).
{
	A = B;
	if (!object_properties_add_key_n(A, K.string.str, K.string.str_len, V))
		PARSE_FAILED;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// default

%type schema_default { Feature * }
%destructor schema_default { feature_unref($$); }

schema_default(A) ::= KEY_DEFAULT instance(V).
{
	A = &jvalue_feature_new(V, validator_set_default)->base;
	if (!A) PARSE_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// jvalue instance

%type instance { jvalue_ref }
%destructor instance { j_release(&$$); }
%type instance_items_list { jvalue_ref }
%destructor instance_items_list { j_release(&$$); }
%type instance_properties_list { jvalue_ref }
%destructor instance_properties_list { j_release(&$$); }

instance(A) ::= OBJ_START instance_properties_list(B) OBJ_END.
{
	A = B;
}

instance_properties_list(A) ::= .
{
	A = jobject_create();
	if (!A) PARSE_FAILED;
}

instance_properties_list(A) ::= instance_properties_list(B) any_object_key(K) instance(V).
{
	A = B;
	raw_buffer key = { .m_str = K.string.str, .m_len = K.string.str_len };
	if (!jobject_put(A, jstring_create_copy(key), V))
		PARSE_FAILED;
}

instance(A) ::= ARR_START instance_items_list(B) ARR_END.
{
	A = B;
}

instance_items_list(A) ::= .
{
	A = jarray_create(NULL);
	if (!A) PARSE_FAILED;
}

instance_items_list(A) ::= instance_items_list(B) instance(V).
{
	A = B;
	if (!jarray_append(A, V)) PARSE_FAILED;
}

instance(A) ::= BOOLEAN(V).
{
	A = jboolean_create(V.boolean);
	if (!A) PARSE_FAILED;
}

instance(A) ::= STRING(V).
{
	raw_buffer raw = { .m_str = V.string.str, .m_len = V.string.str_len };
	A = jstring_create_copy(raw);
	if (!A) PARSE_FAILED;
}

instance(A) ::= NUMBER(V).
{
	raw_buffer raw = { .m_str = V.string.str, .m_len = V.string.str_len };
	A = jnumber_create(raw);
	if (!A) PARSE_FAILED;
}

instance(A) ::= NULL.
{
	A = jnull();
}
