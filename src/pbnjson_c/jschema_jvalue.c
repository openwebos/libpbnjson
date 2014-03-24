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
 *  @file jschema_jvalue.c
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include <jschema.h>

#include "jtraverse.h"
#include "jschema_types_internal.h"
#include "validation/schema_builder.h"

static bool schema_null(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_NULL); }

static bool schema_start_map(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_OBJ_START); }

static bool schema_end_map(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_OBJ_END); }

static bool schema_start_arr(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_ARR_START); }

static bool schema_end_arr(void *ctx)
{ return jschema_builder_token((jschema_builder *)ctx, TOKEN_ARR_END); }

static bool schema_bool(void *ctx, bool value)
{ return jschema_builder_bool((jschema_builder *)ctx, value); }

static bool schema_str(void *ctx, const unsigned char *str, size_t len)
{ return jschema_builder_str((jschema_builder *)ctx, (const char *)str, len); }

static bool schema_key(void *ctx, const unsigned char *str, size_t len)
{ return jschema_builder_key((jschema_builder *)ctx, (const char *)str, len); }

static bool schema_number(void *ctx, const char *buf, size_t len )
{ return jschema_builder_number((jschema_builder *)ctx, buf, len); }

static bool schema_int(void *ctx, int64_t num)
{
	/* we know exactly what we convert */
	char buf[24];
	int len = sprintf(buf, "%" PRIi64, num);
	assert( 0 < len && len < sizeof(buf) ); /* if len > sizeof(buf) stack already overwritten */
	return schema_number(ctx, buf, (size_t)len);
}

static bool schema_double(void *ctx, double num)
{
	/* we know exactly what we convert */
	char buf[32]; /* hope we'll fit everything */
	int len = sprintf(buf, "%e", num);
	assert( 0 < len && len < sizeof(buf) ); /* if len > sizeof(buf) stack already overwritten */
	return schema_number(ctx, buf, (size_t)len);
}

static void dummy_jarray(void *ctxt, jvalue_ref jref)
{}

jschema_ref jschema_parse_jvalue(jvalue_ref value, JErrorCallbacksRef errorHandler, const char *root_scope)
{
	jschema_ref schema = jschema_new();
	if (!schema)
		return NULL;

	jschema_builder builder;
	jschema_builder_init(&builder);

	static struct TraverseCallbacks traverse = {
		.jnull = schema_null,
		.jbool = schema_bool,
		.jnumber_int = schema_int,
		.jnumber_double = schema_double,
		.jnumber_raw = schema_number,
		.jstring = schema_str,
		.jobj_start = schema_start_map,
		.jobj_key = schema_key,
		.jobj_end = schema_end_map,
		.jarr_start = schema_start_arr,
		.jarr_end = schema_end_arr,
		.jarray = dummy_jarray,
	};

	if (!jvalue_traverse(value, &traverse, &builder))
	{
		jschema_builder_destroy(&builder);
		jschema_release(&schema);
		return NULL;
	}

	schema->validator = jschema_builder_finish(&builder, schema->uri_resolver, root_scope);

	if (!schema->validator)
	{
		if (errorHandler && errorHandler->m_parser)
		{
			struct __JSAXContext fake_sax_ctxt = {
				.m_errors = errorHandler,
				.m_error_code = jschema_builder_error_code(&builder),
				.errorDescription = (char *) jschema_builder_error_str(&builder),
			};
			errorHandler->m_parser(errorHandler->m_ctxt, &fake_sax_ctxt);
		}
		jschema_builder_destroy(&builder);
		jschema_release(&schema);
		return NULL;
	}

	jschema_builder_destroy(&builder);
	return schema;
}
