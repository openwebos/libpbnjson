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


#include <jobject.h>

#include "liblog.h"
#include "jobject_internal.h"
#include "gen_stream.h"


static bool jvalue_to_string_append (jvalue_ref jref, JStreamRef generating);

static const char *jvalue_tostring_internal_layer2 (jvalue_ref val, JSchemaInfoRef schemainfo, bool schemaNecessary)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER_RETURN_VALUE(val, "null");

	if (!val->m_toString) {

		if (schemaNecessary && !jvalue_check_schema(val, schemainfo)) {
			return NULL;
		}

		bool parseok = false;
		StreamStatus error;
		JStreamRef generating = jstreamInternal(TOP_None);
		if (generating == NULL) {
			return NULL;
		}
		parseok = jvalue_to_string_append (val, generating);
		val->m_toString = generating->finish (generating, &error);
		val->m_toStringDealloc = free;
		assert (val->m_toString != NULL);
		if(!parseok) {
			return NULL;
		}
	}

	return val->m_toString;
}

//Helper function for jobject_to_string_append()
static bool key_value_to_string_append (jobject_key_value key_value, JStreamRef generating)
{
	//jvalue_to_string_append is enough for the key if schema validation isn't needed.
	//->o_key is called for validation.
	//jvalue_to_string_append (key_value.key, generating);

	if(generating->o_key(generating, jstring_deref(key_value.key)->m_data))
	{
		//Key was OK - now process the value.
		if(UNLIKELY(!jvalue_to_string_append (key_value.value, generating)))
		{
			return false;
		}
	}
	else
	{
		PJ_LOG_ERR("Schema validation error with key: '%s'", jstring_get_fast(key_value.key).m_str);
		return false;
	}
	return true;
}

static bool jobject_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	SANITY_CHECK_POINTER(jref);

	if (UNLIKELY(!generating->o_begin (generating))) {
		PJ_LOG_ERR("Schema validation error, objects are not allowed.");
		return false;
	}
	if (!jis_object (jref)) {
		const char *asStr = jvalue_tostring_internal_layer2 (jref, NULL, false);
		generating->string (generating, J_CSTR_TO_BUF("Internal error - not an object"));
		generating->string (generating, j_cstr_to_buffer(asStr));
		// create invalid JSON on purpose
		return false;
	}

	jobject_iter it;
	jobject_iter_init(&it, jref);
	jobject_key_value key_value;
	while (jobject_iter_next(&it, &key_value))
	{
		assert(jis_string(key_value.key));
		if(UNLIKELY(!key_value_to_string_append(key_value, generating)))
		{
			return false;
		}
	}

	if (UNLIKELY(!generating->o_end (generating))) {
		PJ_LOG_ERR("Schema validation error, object did not validate against schema");
		return false;
	}

	return true;
}

static bool jarray_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	ssize_t i;
	assert(jis_array(jref));

	if (UNLIKELY(!generating)) {
		PJ_LOG_ERR("Cannot append string value to NULL output stream");
		return false;
	}

	SANITY_CHECK_POINTER(jref);
	if (UNLIKELY(jis_null(jref))) {
		PJ_LOG_ERR("INTERNAL ERROR!!!!!!!!!! - used internal API for array --> string for JSON null");
		generating->null_value (generating);
		return false;
	}

	if (UNLIKELY(!generating->a_begin (generating))) {
		PJ_LOG_ERR("Schema validation error, arrays are not allowed");
		return false;
	}
	for (i = 0; i < jarray_size (jref); i++) {
		jvalue_ref toAppend = jarray_get (jref, i);
		SANITY_CHECK_POINTER(toAppend);
		if (UNLIKELY(!jvalue_to_string_append (toAppend, generating))) {
			return false;
		}
	}
	if (UNLIKELY(!generating->a_end (generating))) {
		PJ_LOG_ERR("Schema validation error, array did not validate against schema");
		return false;
	}

	return true;
}

static bool jnumber_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	SANITY_CHECK_POINTER(jref);
	if (jnum_deref(jref)->m_error) {
		PJ_LOG_WARN("converting a number that has an error (%d) set to a string", jnum_deref(jref)->m_error);
	}

	bool ok = false;
	switch (jnum_deref(jref)->m_type) {
		case NUM_RAW:
			assert(jnum_deref(jref)->value.raw.m_len != 0);
			ok = (generating->number (generating, jnum_deref(jref)->value.raw) != NULL);
			break;
		case NUM_FLOAT:
			ok = (generating->floating (generating, jnum_deref(jref)->value.floating) != NULL);
			break;
		case NUM_INT:
			ok = (generating->integer (generating, jnum_deref(jref)->value.integer) != NULL);
			break;
		default:
			// mismatched on purpose so that generation yields an error
			assert(false);
			generating->o_begin (generating);
			raw_buffer asStrBuf = J_CSTR_TO_BUF("Error - Unrecognized number type");
			generating->string (generating, asStrBuf);
			generating->integer (generating, jnum_deref(jref)->m_type);
			break;
	}

	return ok;
}

static inline bool jstring_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	bool result = (generating->string (generating, jstring_deref(jref)->m_data) != NULL);
	if (UNLIKELY(!result)) {
		PJ_LOG_ERR("Schema validation error, string '%s' did not validate against schema", jstring_deref(jref)->m_data.m_str);
	}
	return result;
}

static inline bool jboolean_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	bool result = (generating->boolean (generating, jboolean_deref(jref)->value) != NULL);
	if (UNLIKELY(!result)) {
		PJ_LOG_ERR("Schema validation error, bool did not validate against schema");
	}
	return result;
}

static bool jvalue_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	SANITY_CHECK_POINTER(jref);
	if (jref == NULL) {
		PJ_LOG_ERR("Internal error.  Using NULL pointer instead of reference to NULL JSON object");
		jref = &JNULL;
	}

	CHECK_POINTER_MSG_RETURN_VALUE(generating, false, "Internal problem due to buffer to append to being null");

	bool success = false;

	switch (jref->m_type) {
		case JV_NULL:
			success = (generating->null_value (generating) != NULL);
			if (UNLIKELY(!success)) {
				PJ_LOG_ERR("Schema validation error, null value not accepted");
			}
			break;
		case JV_OBJECT:
			success = jobject_to_string_append (jref, generating);
			break;
		case JV_ARRAY:
			success = jarray_to_string_append (jref, generating);
			break;
		case JV_NUM:
			success = jnumber_to_string_append (jref, generating);
			break;
		case JV_STR:
			success = jstring_to_string_append (jref, generating);
			break;
		case JV_BOOL:
			success = jboolean_to_string_append (jref, generating);
			break;
		default:
			PJ_LOG_ERR("Internal error. Unknown jvalue type");
			break;
	}

	return success;
}


static const char *jvalue_tostring_internal_layer1 (jvalue_ref val, JSchemaInfoRef schemainfo, bool schemaNecessary)
{
	if (val->m_toStringDealloc)
		val->m_toStringDealloc(val->m_toString);
	val->m_toString = NULL;

	const char* result = jvalue_tostring_internal_layer2 (val, schemainfo, schemaNecessary);

	if (result == NULL) {
		PJ_LOG_ERR("Failed to generate string from jvalue. Error location: %s", val->m_toString);
	}

	return result;

}

//if schemainfo->m_schema is null, schema_info_all() is used
const char * jvalue_tostring_schemainfo (jvalue_ref val, const JSchemaInfoRef schemainfo)
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

const char * jvalue_tostring (jvalue_ref val, const jschema_ref schema)
{
	JSchemaInfo schemainfo;
	JSchemaResolverRef resolver = NULL;
	JErrorCallbacksRef errors = NULL;

	jschema_info_init(&schemainfo, schema, resolver, errors);

	return jvalue_tostring_internal_layer1(val, &schemainfo, true);
}
