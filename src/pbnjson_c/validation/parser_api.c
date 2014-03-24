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

#include "parser_api.h"
#include "validator.h"
#include "../yajl_compat.h"
#include <yajl/yajl_parse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schema_builder.h"

// YAJL and gperf are used to produce lexical tokens.
// The parser is generated from a EBNF source with LEMON LALR(1) parser (see
// json_schema_grammar.y). The abstract syntax tree is formed using the class
// SchemaParsing for every parsed schema.
//
// Post-parse processing consists of the following steps:
//  * Apply features: gathered features are moved to the type validator
//    for every SchemaParsing.
//  * Combine validators: combine together validators in combinators and
//    the type validator for every SchemaParsing.
//  * Collect URI: resolve URI scopes, and put validators worth remembering
//    into UriResolver.
//  * Finalize parse: Substitute all the SchemaParsing in the tree by their
//    type validators.

// Every YAJL callback means a token for the parser.
static int schema_null(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_NULL); }

static int schema_start_map(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_OBJ_START); }

static int schema_end_map(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_OBJ_END); }

static int schema_start_arr(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_ARR_START); }

static int schema_end_arr(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_ARR_END); }

static int schema_bool(void *ctx, int boolean)
{ return jschema_builder_bool((jschema_builder *)ctx, boolean); }

static int schema_str(void *ctx, const unsigned char *str, size_t len)
{ return jschema_builder_str((jschema_builder *)ctx, (const char *)str, len); }

static int schema_number(void *ctx, const char *str, size_t len)
{ return jschema_builder_number((jschema_builder *)ctx, str, len); }

static int schema_key(void *ctx, const unsigned char *str, size_t len)
{ return jschema_builder_key((jschema_builder *)ctx, (const char *)str, len); }

static yajl_callbacks callbacks =
{
	.yajl_null          = schema_null,
	.yajl_boolean       = schema_bool,
	.yajl_integer       = NULL,
	.yajl_double        = NULL,
	.yajl_number        = schema_number,
	.yajl_string        = schema_str,
	.yajl_start_map     = schema_start_map,
	.yajl_map_key       = schema_key,
	.yajl_end_map       = schema_end_map,
	.yajl_start_array   = schema_start_arr,
	.yajl_end_array     = schema_end_arr
};

Validator* parse_schema_n(char const *str, size_t len,
                          UriResolver *uri_resolver, char const *root_scope,
                          JschemaErrorFunc error_func, void *error_ctxt)
{
	//JsonSchemaParserTrace(stdout, ">>> ");
	jschema_builder yajl_context;
	jschema_builder_init(&yajl_context);

	const bool allow_comments = true;

#if YAJL_VERSION < 20000
	yajl_parser_config yajl_opts =
	{
		allow_comments,
		0,
	};
	yajl_handle yh = yajl_alloc(&callbacks, &yajl_opts, NULL, &yajl_context);
#else
	yajl_handle yh = yajl_alloc(&callbacks, NULL, &yajl_context);

	yajl_config(yh, yajl_allow_comments, allow_comments ? 1 : 0);
	yajl_config(yh, yajl_dont_validate_strings, 1);
#endif // YAJL_VERSION
	if (!yh)
	{
		jschema_builder_destroy(&yajl_context);
		return NULL;
	}

	if (yajl_status_ok != yajl_parse(yh, (const unsigned char *)str, len))
	{
		if (jschema_builder_is_ok(&yajl_context))
		{
			unsigned char *err = yajl_get_error(yh, 0/*verbose*/, (const unsigned char *)str, len);
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh), SEC_SYNTAX, (const char *) err, error_ctxt);
			yajl_free_error(yh, err);
		}
		else
		{
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh), jschema_builder_error_code(&yajl_context),
				           jschema_builder_error_str(&yajl_context), error_ctxt);
		}
		yajl_free(yh);
		jschema_builder_destroy(&yajl_context);
		return NULL;
	}

#if YAJL_VERSION < 20000
	if (yajl_status_ok != yajl_parse_complete(yh))
#else
	if (yajl_status_ok != yajl_complete_parse(yh))
#endif
	{
		if (jschema_builder_is_ok(&yajl_context))
		{
			unsigned char *err = yajl_get_error(yh, 0, (const unsigned char *)str, len);
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh), SEC_SYNTAX, (const char *) err, error_ctxt);
			yajl_free_error(yh, err);
		}
		else
		{
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh), jschema_builder_error_code(&yajl_context),
				           jschema_builder_error_str(&yajl_context), error_ctxt);
		}
		yajl_free(yh);
		jschema_builder_destroy(&yajl_context);
		return NULL;
	}

	Validator *v = jschema_builder_finish(&yajl_context, uri_resolver, root_scope);
	if (v == NULL) /* some error happened? */
	{
		if (error_func)
			error_func(yajl_get_bytes_consumed(yh), jschema_builder_error_code(&yajl_context),
			           jschema_builder_error_str(&yajl_context), error_ctxt);
		yajl_free(yh);
		jschema_builder_destroy(&yajl_context);
		return NULL;
	}
	yajl_free(yh);

	jschema_builder_destroy(&yajl_context);
	return v;
}

Validator* parse_schema(char const *str,
                        UriResolver *uri_resolver, char const *root_scope,
                        JschemaErrorFunc error_func, void *error_ctxt)
{
	return parse_schema_n(str, strlen(str),
	                      uri_resolver, root_scope,
	                      error_func, error_ctxt);
}

Validator* parse_schema_no_uri(char const *str,
                               JschemaErrorFunc error_func, void *error_ctxt)
{
	return parse_schema(str, NULL, NULL, error_func, error_ctxt);
}

Validator* parse_schema_bare(char const *str)
{
	return parse_schema_no_uri(str, NULL, NULL);
}
