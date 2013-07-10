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

#include "gen_stream.h"
#include "jobject.h"
#include "jcallbacks.h"
#include "jschema_internal.h"
#include "jschema_types_internal.h"
#include "jparse_stream_internal.h"

#include <yajl/yajl_gen.h>
#include "yajl_compat.h"

#include <stdio.h>
#include <string.h>
#include <sys_malloc.h>
#include <assert.h>
#include <inttypes.h>

#include <compiler/malloc_attribute.h>
#include <compiler/unused_attribute.h>

#include "liblog.h"

#define DEREF_CALLBACK(callback, ...) \
	do { \
		if (callback != NULL) \
			if (callback(__VA_ARGS__) == 0) return NULL; \
	} while (0)

#define ERR_HANDLER_FAILED(err_handler, cb, ...) \
	(err_handler) == NULL ||  (err_handler->cb) == NULL || !((err_handler->cb)(err_handler->m_ctxt, ##__VA_ARGS__))

#define SCHEMA_HANDLER_FAILED(ctxt) ERR_HANDLER_FAILED((ctxt)->m_errors, m_schema, (ctxt))

typedef struct PJSON_LOCAL {
	struct __JStream stream;
	TopLevelType opened;
	yajl_gen handle;
	StreamStatus error;
	JSAXContextRef ctxt;
} ActualStream;

#define CHECK_HANDLE(stream) 							\
	do {									\
		if (stream->error != GEN_OK || stream->handle == NULL) {	\
			if (stream->error == GEN_OK) {				\
				stream->error = GEN_GENERIC_ERROR;		\
			}							\
			return stream;						\
		}								\
	} while(0)

// Why is this emitting a warning - is this due to the compiler version on OSX?
#if 0
#define INTERNAL_MALLOC_SIZE RETURN_SIZE2(2, 3)
#define INTERNAL_MALLOC_SIZE2 RETURN_SIZE(3)
#else
#define INTERNAL_MALLOC_SIZE
#define INTERNAL_MALLOC_SIZE2
#endif

// for some reason the return size is ignored
static void * pjson_internal_calloc(UNUSED_VAR void *ctx, size_t nmemb, size_t sz) MALLOC_FUNC INTERNAL_MALLOC_SIZE UNUSED_FUNC;
static void * pjson_internal_malloc(UNUSED_VAR void *ctx, size_t nmemb, size_t sz) MALLOC_FUNC INTERNAL_MALLOC_SIZE UNUSED_FUNC;
static void * pjson_internal_realloc(UNUSED_VAR void *ctx, void *ptr, unsigned int sz) UNUSED_FUNC INTERNAL_MALLOC_SIZE2;

static void pjson_internal_free(UNUSED_VAR void *ctx, void * ptr) UNUSED_FUNC;

static void * pjson_internal_calloc(void *ctx, size_t nmemb, size_t sz)
{
	return calloc(nmemb, sz);
}

static void * pjson_internal_malloc(void *ctx, size_t nmemb, size_t sz)
{
	return malloc(sz * nmemb);
}

static void * pjson_internal_realloc(void *ctx, void *ptr, unsigned int sz)
{
	return realloc(ptr, sz);
}

static void pjson_internal_free(void *ctx, void * ptr)
{
	return free(ptr);
}

static ActualStream* begin_object_simple(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	yajl_gen_map_open(stream->handle);
	return stream;
}

static ActualStream* key_object_simple(ActualStream* stream, raw_buffer buf)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	SANITY_CHECK_POINTER(buf.m_str);
	yajl_gen_string(stream->handle, (const unsigned char *)buf.m_str, buf.m_len);
	return stream;
}


static ActualStream* end_object_simple(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	yajl_gen_map_close(stream->handle);
	return stream;
}

static ActualStream* begin_array_simple(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	yajl_gen_array_open(stream->handle);
	return stream;
}

static ActualStream* end_array_simple(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	yajl_gen_array_close(stream->handle);
	return stream;
}

static ActualStream* val_num_simple(ActualStream* stream, raw_buffer numstr)
{
	SANITY_CHECK_POINTER(stream);
	SANITY_CHECK_POINTER(numstr.m_str);
	assert (numstr.m_str != NULL);
	CHECK_HANDLE(stream);
	yajl_gen_number(stream->handle, numstr.m_str, numstr.m_len);
	return stream;
}

static ActualStream* val_int_simple(ActualStream* stream, int64_t number)
{
	char buf[24];
	int printed;
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	printed = snprintf(buf, sizeof(buf), "%" PRId64, number);
	yajl_gen_number(stream->handle, buf, printed);
	return stream;
}

static ActualStream* val_dbl_simple(ActualStream* stream, double number)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	// yajl doesn't print properly (%g doesn't seem to do what it claims to
	// do or something - fails for 42323.0234234)
	// let's work around it with the  raw interface by
	char f[32];
	int len = snprintf(f, sizeof(f) - 1, "%.14lg", number);
	yajl_gen_number(stream->handle, f, len);
	return stream;
}

static ActualStream* val_str_simple(ActualStream* stream, raw_buffer str)
{
	SANITY_CHECK_POINTER(stream);
	SANITY_CHECK_POINTER(str.m_str);
	assert(str.m_str != NULL);
	CHECK_HANDLE(stream);
	yajl_gen_string(stream->handle, (const unsigned char *)str.m_str, str.m_len);

	return stream;
}

static ActualStream* val_bool_simple(ActualStream* stream, bool boolean)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	yajl_gen_bool(stream->handle, boolean);
	return stream;
}

static ActualStream* val_null_simple(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	yajl_gen_null(stream->handle);
	return stream;
}

static ActualStream* begin_object(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_obj(stream->ctxt, stream->ctxt->m_validation)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif
	yajl_gen_map_open(stream->handle);
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_start_map, stream->ctxt);

	return stream;
}

static ActualStream* key_object(ActualStream* stream, raw_buffer buf)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	SANITY_CHECK_POINTER(buf.m_str);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_key(stream->ctxt, stream->ctxt->m_validation, j_str_to_buffer((char *)buf.m_str, buf.m_len))) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
			{
				return NULL;
			}
		}
	}
#endif
	yajl_gen_string(stream->handle, (const unsigned char *)buf.m_str, buf.m_len);
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_map_key, stream->ctxt, (const unsigned char *)buf.m_str, buf.m_len);

	return stream;
}

static ActualStream* end_object(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_obj_end(stream->ctxt, stream->ctxt->m_validation)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
			{
				return NULL;
			}
		}
	}
#endif
	yajl_gen_map_close(stream->handle);
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_end_map, stream->ctxt);

	return stream;
}

static ActualStream* begin_array(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_arr(stream->ctxt, stream->ctxt->m_validation)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif
	yajl_gen_array_open(stream->handle);

	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_start_array, stream->ctxt);

	return stream;
}

static ActualStream* end_array(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_arr_end(stream->ctxt, stream->ctxt->m_validation)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif
	yajl_gen_array_close(stream->handle);

	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_end_array, stream->ctxt);

	return stream;
}

static ActualStream* val_num(ActualStream* stream, raw_buffer numstr)
{
	SANITY_CHECK_POINTER(stream);
	SANITY_CHECK_POINTER(numstr.m_str);
	assert (numstr.m_str != NULL);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_num(stream->ctxt, stream->ctxt->m_validation, numstr)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt)) {
				return NULL;
			}
		}
	}
#endif
	yajl_gen_number(stream->handle, numstr.m_str, numstr.m_len);
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_number, stream->ctxt, numstr.m_str, numstr.m_len);

	return stream;
}

static ActualStream* val_int(ActualStream* stream, int64_t number)
{
	char buf[24];
	int printed;
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	printed = snprintf(buf, sizeof(buf), "%" PRId64, number);
#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_num(stream->ctxt, stream->ctxt->m_validation, j_str_to_buffer(buf, printed))) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif
	yajl_gen_number(stream->handle, buf, printed);
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_number, stream->ctxt, buf, printed);

	return stream;
}

static ActualStream* val_dbl(ActualStream* stream, double number)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);
	// yajl doesn't print properly (%g doesn't seem to do what it claims to
	// do or something - fails for 42323.0234234)
	// let's work around it with the  raw interface by 
	char f[32];
	int len = snprintf(f, sizeof(f) - 1, "%.14lg", number);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_num(stream->ctxt, stream->ctxt->m_validation, j_str_to_buffer(f, len))) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif

	yajl_gen_number(stream->handle, f, len);
#ifdef _DEBUG
	const unsigned char *buffer;
	unsigned int bufLen;
	yajl_gen_get_buf(stream->handle, &buffer, &bufLen);
#endif
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_number, stream->ctxt, f, len);

	return stream;
}

static ActualStream* val_str(ActualStream* stream, raw_buffer str)
{
	SANITY_CHECK_POINTER(stream);
	SANITY_CHECK_POINTER(str.m_str);
	assert(str.m_str != NULL);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_str(stream->ctxt, stream->ctxt->m_validation, str)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt)) //Calls callback stream->ctxt->m_errors->m_schema.
			{
				return NULL;
			}
		}
	}
#endif
	yajl_gen_string(stream->handle, (const unsigned char *)str.m_str, str.m_len);
	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_string, stream->ctxt, (const unsigned char *)str.m_str, str.m_len);

	return stream;
}

static ActualStream* val_bool(ActualStream* stream, bool boolean)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_bool(stream->ctxt, stream->ctxt->m_validation, boolean)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif

	yajl_gen_bool(stream->handle, boolean);

	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_boolean, stream->ctxt, boolean);

	return stream;
}

static ActualStream* val_null(ActualStream* stream)
{
	SANITY_CHECK_POINTER(stream);
	CHECK_HANDLE(stream);

#if !BYPASS_SCHEMA
#if SHORTCUT_SCHEMA_ALL
	if (stream->ctxt->m_validation->m_state->m_schema != jschema_all())
#endif
	{
		if (!jschema_null(stream->ctxt, stream->ctxt->m_validation)) {
			if (SCHEMA_HANDLER_FAILED(stream->ctxt))
				return NULL;
		}
	}
#endif

	yajl_gen_null(stream->handle);

	DEREF_CALLBACK(stream->ctxt->m_handlers->yajl_null, stream->ctxt);

	return stream;
}

static StreamStatus convert_error_code(yajl_gen_status raw_code)
{
	switch (raw_code) {
		case yajl_gen_generation_complete:
		case yajl_gen_status_ok:
			return GEN_OK;
		case yajl_gen_keys_must_be_strings:
			return GEN_KEYS_MUST_BE_STRINGS;
		case yajl_max_depth_exceeded:
		case yajl_gen_in_error_state:
		default:
			return GEN_GENERIC_ERROR;
	}
}

static void destroy_stream(ActualStream* stream)
{
	if(stream->handle)
		yajl_gen_free(stream->handle);
	SANITY_KILL_POINTER(stream->handle);

	if(stream->ctxt)
	{
		dom_cleanup_from_jsax(stream->ctxt);

		if(stream->ctxt->m_validation)
			validation_destroy(&stream->ctxt->m_validation);
		SANITY_KILL_POINTER(stream->ctxt->m_validation);

		if(stream->ctxt->m_errorstate)
			jsax_error_release(&stream->ctxt->m_errorstate);
		SANITY_KILL_POINTER(stream->ctxt->m_errorstate);

		if(stream->ctxt->m_errors)
			free(stream->ctxt->m_errors);
		SANITY_KILL_POINTER(stream->ctxt->m_errors);

		free(stream->ctxt);
		SANITY_KILL_POINTER(stream->ctxt);
	}

	if(stream)
		free(stream);
	SANITY_KILL_POINTER(stream);
}

static char* finish_stream(ActualStream* stream, StreamStatus *error_code)
{
	char *buf = NULL;
	yajl_size_t len;
	yajl_gen_status result;

	SANITY_CHECK_POINTER(stream);
	SANITY_CHECK_POINTER(error_code);

	switch (stream->opened) {
		case TOP_None:
			break;
		case TOP_Object:
			end_object(stream);
			break;
		case TOP_Array:
			end_array(stream);
			break;
		default:
			PJ_LOG_ERR("Invalid object type: %d", stream->opened);
			if (error_code) *error_code = GEN_GENERIC_ERROR;
			goto stream_error;
	}

	if (!stream->handle) {
		if (error_code) *error_code = GEN_GENERIC_ERROR;
		goto stream_error;
	}

	if (stream->error == GEN_OK) {
		const unsigned char *yajlBuf;
		result = yajl_gen_get_buf(stream->handle, &yajlBuf, &len);
		if (error_code) {
			*error_code = convert_error_code(result);
		}
		if (result != yajl_gen_status_ok && result != yajl_gen_generation_complete) {
			buf = NULL;
		} else {
			buf = calloc(len + 1, sizeof(char));
			if (LIKELY(buf != NULL)) {
				memcpy(buf, yajlBuf, len);
			}
		}
	} else if (error_code) {
		*error_code = stream->error;
	}

	destroy_stream(stream);

	return buf;

stream_error:
	destroy_stream(stream);

	return NULL;
}

static struct __JStream yajl_stream_generator =
{
	(jObjectBegin)begin_object,
	(jObjectKey)key_object,
	(jObjectEnd)end_object,
	(jArrayBegin)begin_array,
	(jArrayEnd)end_array,
	(jNumber)val_num,
	(jNumberI)val_int,
	(jNumberF)val_dbl,
	(jString)val_str,
	(jBoolean)val_bool,
	(jNull)val_null,
	(jFinish)finish_stream
};

static struct __JStream yajl_stream_generator_simple =
{
	(jObjectBegin)begin_object_simple,
	(jObjectKey)key_object_simple,
	(jObjectEnd)end_object_simple,
	(jArrayBegin)begin_array_simple,
	(jArrayEnd)end_array_simple,
	(jNumber)val_num_simple,
	(jNumberI)val_int_simple,
	(jNumberF)val_dbl_simple,
	(jString)val_str_simple,
	(jBoolean)val_bool_simple,
	(jNull)val_null_simple,
	(jFinish)finish_stream
};


static yajl_callbacks yajl_cb =
{
	(pj_yajl_null)dom_null,
	(pj_yajl_boolean)dom_boolean, // yajl_boolean
	NULL, // yajl_integer
	NULL, // yajl_double
	(pj_yajl_number)dom_number, // yajl_number
	(pj_yajl_string)dom_string, // yajl_stirng
	(pj_yajl_start_map)dom_object_start, // yajl_start_map
	(pj_yajl_map_key)dom_object_key, // yajl_map_key
	(pj_yajl_end_map)dom_object_end, // yajl_end_map
	(pj_yajl_start_array)dom_array_start, // yajl_start_array
	(pj_yajl_end_array)dom_array_end, // yajl_end_array
};

//These error callbacks return true only if they are able to fix the problem.
static bool handleParserError(void *ctxt, JSAXContextRef parseCtxt)
{
	return false;
}

static bool handleSchemaError(void *ctxt, JSAXContextRef parseCtxt)
{
	return false;
}

static bool handleUnknownError(void *ctxt, JSAXContextRef parseCtxt)
{
	return false;
}

static JErrorCallbacksRef create_default_error_handlers()
{
	JErrorCallbacksRef error_handler = (JErrorCallbacksRef)malloc(sizeof(struct JErrorCallbacks));

	if (error_handler == NULL)
		return NULL;


	error_handler->m_parser = handleParserError;
	error_handler->m_schema = handleSchemaError;
	error_handler->m_unknown = handleUnknownError;

	return error_handler;
}

JStreamRef jstreamInternal(jschema_ref schema, TopLevelType type, bool schemaNecessary)
{
	JSchemaInfo schemainfo;
	JSchemaResolverRef resolver = jget_garbage_resolver();
	JErrorCallbacksRef errors = NULL;

	jschema_info_init(&schemainfo, schema, resolver, errors);

	return jstreamInternalWithInfo(&schemainfo, type, schemaNecessary);
}

JStreamRef jstreamInternalWithInfo(JSchemaInfoRef schemainfo, TopLevelType type, bool schemaNecessary)
{
	ActualStream* stream = (ActualStream*)calloc(1, sizeof(ActualStream));
	if (UNLIKELY(stream == NULL)) {
		return NULL;
	}
	if (schemaNecessary)
		memcpy(&stream->stream, &yajl_stream_generator, sizeof(struct __JStream));
	else
		memcpy(&stream->stream, &yajl_stream_generator_simple, sizeof(struct __JStream));

#if 0
	// try to use custom allocators to bypass freeing the buffer & instead passing off
	// ownership to the caller.  for now, this is too difficult - we'll duplicate the string instead
	yajl_alloc_funcs allocators = {
		pjson_internal_malloc,
		pjso_internal_malloc,
		pjso_internal_free,
		NULL,
	};
	stream->handle = yajl_gen_alloc(NULL, &allocators);
#else

#if YAJL_VERSION < 20000
	stream->handle = yajl_gen_alloc(NULL, NULL);
#else
	stream->handle = yajl_gen_alloc(NULL);
#endif

#endif
	stream->opened = type;
	stream->error = GEN_OK;

	if(schemainfo->m_errHandler == NULL)
	{
		schemainfo->m_errHandler = create_default_error_handlers();
	}

	JSAXContextRef ctxt = (JSAXContextRef)malloc(sizeof(struct __JSAXContext));
	DomInfo *domctxt = calloc(1, sizeof(struct DomInfo));
	ctxt->ctxt = domctxt;
	ctxt->m_handlers = &yajl_cb;

	ctxt->m_validation = jschema_init(schemainfo);
	if (ctxt->m_validation == NULL) {
		return NULL;
	}
	ctxt->m_errors = schemainfo->m_errHandler;
	ctxt->m_errorstate = jsax_error_init();
	if (schemainfo->m_errHandler != NULL)
		schemainfo->m_errHandler->m_ctxt = ctxt;
	stream->error = GEN_OK;
	stream->ctxt = ctxt;

	return (JStreamRef)stream;
}

JStreamRef jstream(jschema_ref schema)
{
	return jstreamInternal(schema, TOP_None, true);
}

JStreamRef jstreamObj(jschema_ref schema)
{
	JStreamRef opened = jstreamInternal(schema, TOP_Object, true);
	opened->o_begin(opened);
	return opened;
}

JStreamRef jstreamArr(jschema_ref schema)
{
	JStreamRef opened = jstreamInternal(schema, TOP_Array, true);
	opened->a_begin(opened);
	return opened;
}
