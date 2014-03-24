/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2014 LG Electronics, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * LICENSE@@@
 ****************************************************************/

/**
 *  @file schema_builder.c
 */

#include <glib.h>

#include "schema_builder.h"
#include "schema_keywords.h"
#include "json_schema_grammar.h"

jschema_builder *jschema_builder_create()
{
	jschema_builder *builder = g_slice_new(jschema_builder);
	jschema_builder_init(builder);
	return builder;
}

void jschema_builder_free(jschema_builder *builder)
{
	jschema_builder_destroy((jschema_builder *)builder);
	g_slice_free(jschema_builder, builder);
}

Validator* jschema_builder_finish(jschema_builder *builder, UriResolver *uri_resolver, char const *root_scope)
{
	// Let the parser finish its job.
	static TokenParam token_param;
	JsonSchemaParser(builder->parser, 0, token_param, &builder->parser_ctxt);

	// Even if parsing was completed there can be an error
	if (builder->parser_ctxt.error != SEC_OK)
	{
		return NULL;
	}
	// Post-parse processing
	Validator *v = builder->parser_ctxt.validator;
	// Move parsed features to the validators
	validator_apply_features(v);
	// Combine type validator and other validators contaiters (allOf, anyOf, etc.)
	validator_combine(v);

	if (uri_resolver)
		validator_collect_uri(v, root_scope, uri_resolver);

	// Substitute every SchemaParsing by its type validator for every node
	// in the AST.
	return validator_finalize_parse(v);
}

bool jschema_builder_key(jschema_builder *builder, const char *str, size_t len)
{
	TokenParam token_param =
	{
		.string = {
			.str = (char const *) str,
			.str_len = len,
		},
	};
	// Object key may mean different tokens for the parser, each schema keyword
	// is a distinct token for the parser.
	const struct JsonSchemaKeyword *k = json_schema_keyword_lookup(str, len);
	JsonSchemaParser(builder->parser,
	                 k ? k->token : TOKEN_KEY_NOT_KEYWORD, token_param,
	                 &builder->parser_ctxt);
	return builder->parser_ctxt.error == SEC_OK;
}
