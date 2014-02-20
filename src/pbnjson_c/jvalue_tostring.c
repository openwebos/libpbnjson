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


#include <jobject.h>

#include "liblog.h"
#include "jobject_internal.h"
#include "jtraverse.h"
#include "gen_stream.h"

static bool to_string_append_jnull(void *ctxt)
{
	JStreamRef generating = (JStreamRef)ctxt;
	bool res = (generating->null_value(generating) != NULL);
	if (!res) {
		PJ_LOG_ERR("PBNJSON_NULL_NOT_ALLOWED", 0, "Schema validation error, null value not accepted");
	}
	return res;
}

//Helper function for jobject_to_string_append()
static bool to_string_append_jkeyvalue(void *ctxt, const unsigned char *str, size_t len)
{
	JStreamRef generating = (JStreamRef)ctxt;
	bool res = generating->o_key(generating, (raw_buffer){(const char*)str, len});
	if (!res) {
		PJ_LOG_ERR("PBNJSON_SCHEMA_VALIDATION_ERR", 1, PMLOGKS("KEY", str), "Schema validation error with key: '%s'", str);
	}
	return res;
}

static bool to_string_append_jobject_start(void *ctxt)
{
	JStreamRef generating = (JStreamRef)ctxt;
	if (!generating->o_begin(generating)) {
		PJ_LOG_ERR("PBNJSON_OBJS_NOT_ALLOWED", 0, "Schema validation error, objects are not allowed.");
		return false;
	}
	return true;
}

static bool to_string_append_jobject_end(void *ctxt)
{
	JStreamRef generating = (JStreamRef)ctxt;

	if (!generating->o_end(generating)) {
		PJ_LOG_ERR("PBNJSON_OBJ_INVALID", 0, "Schema validation error, object did not validate against schema");
		return false;
	}
	return true;
}

static bool to_string_append_jarray_start(void *ctxt)
{
	JStreamRef generating = (JStreamRef)ctxt;
	if (!generating->a_begin(generating)) {
		PJ_LOG_ERR("PBNJSON_ARR_NOT_ALLOWED", 0, "Schema validation error, arrays are not allowed");
		return false;
	}
	return true;
}

static bool to_string_append_jarray_end(void *ctxt)
{
	JStreamRef generating = (JStreamRef)ctxt;
	if (!generating->a_end(generating)) {
		PJ_LOG_ERR("PBNJSON_ARR_INVALID", 0, "Schema validation error, array did not validate against schema");
		return false;
	}
	return true;
}

static bool to_string_append_jnumber_raw(void *ctxt, const char *num, size_t len)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->number(generating, (raw_buffer){num, len}) != NULL;
}

static bool to_string_append_jnumber_double(void *ctxt, double num)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->floating(generating, num) != NULL;
}

static bool to_string_append_jnumber_int(void *ctxt, int64_t num)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->integer(generating, num) != NULL;
}

static inline bool to_string_append_jstring(void *ctxt, const unsigned char *str, size_t len)
{
	JStreamRef generating = (JStreamRef)ctxt;
	bool result = (generating->string(generating, (raw_buffer){(const char*)str, len}) != NULL);
	if (!result) {
		PJ_LOG_ERR("PBNJSON_STR_INVALID", 1, PMLOGKS("STRING", str), "Schema validation error, string '%s' did not validate against schema", str);
	}
	return result;
}

static inline bool to_string_append_jbool(void *ctxt, bool value)
{
	JStreamRef generating = (JStreamRef)ctxt;
	bool result = (generating->boolean(generating, value) != NULL);
	if (!result) {
		PJ_LOG_ERR("PBNJSON_BOOL_INVALID", 0, "Schema validation error, bool did not validate against schema");
	}
	return result;
}

static void dummy_jarray(void *ctxt, jvalue_ref jref){}

static struct TraverseCallbacks traverse = {
	to_string_append_jnull,
	to_string_append_jbool,
	to_string_append_jnumber_int,
	to_string_append_jnumber_double,
	to_string_append_jnumber_raw,
	to_string_append_jstring,
	to_string_append_jobject_start,
	to_string_append_jkeyvalue,
	to_string_append_jobject_end,
	to_string_append_jarray_start,
	to_string_append_jarray_end,
	dummy_jarray
};

static const char *jvalue_tostring_internal_layer2(jvalue_ref val, JSchemaInfoRef schemainfo, bool schemaNecessary)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER_RETURN_VALUE(val, "null");

	if (!val->m_toString) {
		if (schemaNecessary && !jvalue_check_schema(val, schemainfo)) {
			return NULL;
		}
		JStreamRef generating = jstreamInternal(TOP_None);
		if (generating == NULL) {
			return NULL;
		}
		bool parseok = jvalue_traverse(val, &traverse, generating);
		StreamStatus error;
		val->m_toString = generating->finish(generating, &error);
		val->m_toStringDealloc = free;
		assert (val->m_toString != NULL);
		if(!parseok) {
			return NULL;
		}
	}

	return val->m_toString;
}

static const char *jvalue_tostring_internal_layer1(jvalue_ref val, JSchemaInfoRef schemainfo, bool schemaNecessary)
{
	if (val->m_toStringDealloc)
		val->m_toStringDealloc(val->m_toString);
	val->m_toString = NULL;

	const char* result = jvalue_tostring_internal_layer2(val, schemainfo, schemaNecessary);

	if (result == NULL) {
		PJ_LOG_ERR("PBNJSON_JVAL_TO_STR_ERR", 1, PMLOGKS("STRING", val->m_toString), "Failed to generate string from jvalue. Error location: %s", val->m_toString);
	}

	return result;
}

//if schemainfo->m_schema is null, schema_info_all() is used
const char *jvalue_tostring_schemainfo(jvalue_ref val, const JSchemaInfoRef schemainfo)
{
	return jvalue_tostring_internal_layer1(val, schemainfo, true);
}

const char *jvalue_tostring_simple(jvalue_ref val)
{
	JSchemaInfo schemainfo;
	JSchemaResolverRef resolver = NULL;
	JErrorCallbacksRef errors = NULL;

	jschema_info_init(&schemainfo, jschema_all(), resolver, errors);

	return jvalue_tostring_internal_layer1(val, &schemainfo, false);
}

const char *jvalue_tostring(jvalue_ref val, const jschema_ref schema)
{
	JSchemaInfo schemainfo;
	JSchemaResolverRef resolver = NULL;
	JErrorCallbacksRef errors = NULL;

	jschema_info_init(&schemainfo, schema, resolver, errors);

	return jvalue_tostring_internal_layer1(val, &schemainfo, true);
}
