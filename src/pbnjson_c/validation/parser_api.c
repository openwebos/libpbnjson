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

#include "parser_api.h"
#include "parser_context.h"
#include "json_schema_grammar.h"
#include "schema_keywords.h"
#include "validator.h"
#include "../yajl_compat.h"
#include <yajl/yajl_parse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void *JsonSchemaParserAlloc(void *(*mallocProc)(size_t));
void JsonSchemaParserFree(
	void *p,                    /* The parser to be deleted */
	void (*freeProc)(void*)     /* Function used to reclaim memory */
);
void JsonSchemaParser(
	void *yyp,                   /* The parser */
	int yymajor,                 /* The major token code number */
	TokenParam yyminor,          /* The value for the token */
	ParserContext *
);
void JsonSchemaParserTrace(FILE *f, char *p);


typedef struct _YajlContext
{
	void *parser;
	ParserContext parser_ctxt;
} YajlContext;

static int on_null(void *ctx)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param = { };
	JsonSchemaParser(yajl_context->parser, TOKEN_NULL, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_boolean(void *ctx, int boolean)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param =
	{
		.boolean = boolean,
	};
	JsonSchemaParser(yajl_context->parser, TOKEN_BOOLEAN, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_number(void *ctx, const char *str, yajl_size_t len)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param =
	{
		.string = {
			.str = (char const *) str,
			.str_len = len,
		},
	};
	JsonSchemaParser(yajl_context->parser, TOKEN_NUMBER, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_string(void *ctx, const unsigned char *str, yajl_size_t len)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param =
	{
		.string = {
			.str = (char const *) str,
			.str_len = len,
		},
	};
	JsonSchemaParser(yajl_context->parser, TOKEN_STRING, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_map_key(void *ctx, const unsigned char *str, yajl_size_t len)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param =
	{
		.string = {
			.str = (char const *) str,
			.str_len = len,
		},
	};
	const struct JsonSchemaKeyword *k = json_schema_keyword_lookup(str, len);
	JsonSchemaParser(yajl_context->parser,
	                 k ? k->token : TOKEN_KEY_NOT_KEYWORD, token_param,
	                 &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_start_map(void *ctx)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param;
	JsonSchemaParser(yajl_context->parser, TOKEN_OBJ_START, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_end_map(void *ctx)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param;
	JsonSchemaParser(yajl_context->parser, TOKEN_OBJ_END, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_start_array(void *ctx)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param;
	JsonSchemaParser(yajl_context->parser, TOKEN_ARR_START, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static int on_end_array(void *ctx)
{
	YajlContext *yajl_context = (YajlContext *) ctx;
	TokenParam token_param;
	JsonSchemaParser(yajl_context->parser, TOKEN_ARR_END, token_param, &yajl_context->parser_ctxt);
	return yajl_context->parser_ctxt.success;
}

static yajl_callbacks callbacks =
{
	.yajl_null          = on_null,
	.yajl_boolean       = on_boolean,
	.yajl_integer       = NULL,
	.yajl_double        = NULL,
	.yajl_number        = on_number,
	.yajl_string        = on_string,
	.yajl_start_map     = on_start_map,
	.yajl_map_key       = on_map_key,
	.yajl_end_map       = on_end_map,
	.yajl_start_array   = on_start_array,
	.yajl_end_array     = on_end_array
};

Validator* parse_schema_n(char const *str, size_t len,
                          UriResolver *uri_resolver, char const *root_scope,
                          JschemaErrorFunc error_func, void *error_ctxt)
{
	//JsonSchemaParserTrace(stdout, ">>> ");
	void *parser = JsonSchemaParserAlloc(malloc);
	YajlContext yajl_context =
	{
		.parser = parser,
		.parser_ctxt = {
			.validator = NULL,
			.success = true,
			.error_message = NULL,
		},
	};

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
	if (yh)
		yajl_config(yh, yajl_allow_comments, allow_comments ? 1 : 0);
#endif // YAJL_VERSION
	if (!yh)
	{
		JsonSchemaParserFree(parser, free);
		return NULL;
	}

	if (yajl_status_ok != yajl_parse(yh, (const unsigned char *)str, len))
	{
		if (yajl_context.parser_ctxt.success)
		{
			unsigned char *err = yajl_get_error(yh, 0/*verbose*/, (const unsigned char *)str, len);
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh), (const char *) err, error_ctxt);
			yajl_free_error(yh, err);
		}
		else
		{
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh),
				           yajl_context.parser_ctxt.error_message, error_ctxt);
		}
		yajl_free(yh);
		JsonSchemaParserFree(parser, free);
		return NULL;
	}

#if YAJL_VERSION < 20000
	if (yajl_status_ok != yajl_parse_complete(yh))
#else
	if (yajl_status_ok != yajl_complete_parse(yh))
#endif
	{
		if (yajl_context.parser_ctxt.success)
		{
			unsigned char *err = yajl_get_error(yh, 0, (const unsigned char *)str, len);
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh), (const char *) err, error_ctxt);
			yajl_free_error(yh, err);
		}
		else
		{
			if (error_func)
				error_func(yajl_get_bytes_consumed(yh),
				           yajl_context.parser_ctxt.error_message, error_ctxt);
		}
		yajl_free(yh);
		JsonSchemaParserFree(parser, free);
		return NULL;
	}

	TokenParam token_param;
	JsonSchemaParser(parser, 0, token_param, &yajl_context.parser_ctxt);

	yajl_free(yh);
	JsonSchemaParserFree(parser, free);

	// Post-parse processing
	Validator *v = yajl_context.parser_ctxt.validator;
	// Move parsed features to the validators
	validator_apply_features(v);
	// Combine type validator and other validators contaiters (allOf, anyOf, etc.)
	validator_combine(v);
	if (uri_resolver)
		validator_collect_uri(v, root_scope, uri_resolver);
	Validator *result = validator_finalize_parse(v);
	validator_unref(v);
	return result;
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
