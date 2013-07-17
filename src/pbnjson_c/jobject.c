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

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <compiler/inline_attribute.h>
#include <compiler/nonnull_attribute.h>
#include <compiler/builtins.h>
#include <math.h>
#include <inttypes.h>

#include <jobject.h>
#include <jschema_internal.h>

#include <sys_malloc.h>
#include <sys/mman.h>
#include "jobject_internal.h"
#include "liblog.h"
#include "jvalue/num_conversion.h"

#ifdef DBG_C_MEM
#define PJ_LOG_MEM(...) PJ_LOG_INFO(__VA_ARGS__)
#else
#define PJ_LOG_MEM(...) do { } while (0)
#endif

#ifdef DBG_C_REFCNT
#define PJ_LOG_REFCNT(...) PJ_LOG_INFO(__VA_ARGS__)
#else
#define PJ_LOG_REFCNT(...) do { } while (0)
#endif

#ifndef PJSON_EXPORT
#error "Compiling with the wrong options"
#endif

#include <jgen_stream.h>
#include "gen_stream.h"

#include "liblog.h"

#define CONST_C_STRING(string) (string), sizeof(string)
#define VAR_C_STRING(string) (string), strlen(string)

#ifndef NDEBUG
static int s_inGdb = 0;
#define NUM_TERM_NULL 1
#else
#define NUM_TERM_NULL 0
#endif

#define TRACE_REF(format, pointer, ...) PJ_LOG_TRACE("TRACE JVALUE_REF: %p " format, pointer, ##__VA_ARGS__)

// 7 NULL bytes is enough to ensure that any Unicode string will be NULL-terminated
// even if it is malformed Unicode
#define SAFE_TERM_NULL_LEN 7

jvalue JNULL = {
	.m_type = JV_NULL,
	.m_refCnt = 1,
	.m_toString = "null",
	.m_toStringDealloc = NULL
};

static jvalue JEMPTY_STR = {
	.value.val_str = {
		.m_dealloc = NULL,
		.m_data = {
			.m_str = "",
			.m_len = 0,
		}
	},
	.m_type = JV_STR,
	.m_refCnt = 1,
	.m_toString = "",
	.m_toStringDealloc = NULL
};

static const char *jvalue_tostring_internal_layer2 (jvalue_ref val, JSchemaInfoRef schemainfo, bool schemaNecessary);
static bool jvalue_to_string_append (jvalue_ref jref, JStreamRef generating);
static bool jobject_to_string_append (jvalue_ref jref, JStreamRef generating);
static bool jarray_to_string_append (jvalue_ref jref, JStreamRef generating);
static bool jnumber_to_string_append (jvalue_ref jref, JStreamRef generating);
static jvalue_ref jnumber_duplicate (jvalue_ref num) NON_NULL(1);
static inline bool jstring_to_string_append (jvalue_ref jref, JStreamRef generating);
static inline bool jboolean_to_string_append (jvalue_ref jref, JStreamRef generating);
static bool jstring_equal_internal(jvalue_ref str, jvalue_ref other) NON_NULL(1, 2);
static inline bool jstring_equal_internal2(jvalue_ref str, raw_buffer *other) NON_NULL(1, 2);
static bool jstring_equal_internal3(raw_buffer *str, raw_buffer *other) NON_NULL(1, 2);

bool jbuffer_equal(raw_buffer buffer1, raw_buffer buffer2)
{
	return buffer1.m_len == buffer2.m_len &&
			memcmp(buffer1.m_str, buffer2.m_str, buffer1.m_len) == 0;
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

/**
 * NOTE: The structure returned (if not null) is always initialized to 0 except for the
 * reference count (1) and the type (set to the first parameter)
 * @param type The type of JSON value to create
 * @return NULL or a reference to a valid, dynamically allocated, structure that isn't a JSON null reference.
 */
static jvalue_ref jvalue_create (JValueType type)
{
	jvalue_ref new_value = (jvalue_ref) calloc (1, sizeof(jvalue));
	CHECK_ALLOC_RETURN_NULL(new_value);
	new_value->m_refCnt = 1;
	new_value->m_type = type;
	TRACE_REF("created", new_value);
	return new_value;
}

#if PJSON_LOG_INFO && !PJSON_NO_LOGGING && DBG_C_REFCNT
#define COUNT_EMPTY_NULL 1
#endif

#if COUNT_EMPTY_NULL
static int jnull_cnt = 0;
static int jempty_cnt = 0;
#endif

jvalue_ref jvalue_copy (jvalue_ref val)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER_RETURN(val);
	assert(s_inGdb || val->m_refCnt > 0);

	if (val == &JNULL) {
#if COUNT_EMPTY_NULL
		PJ_LOG_REFCNT("attempt to grab ownership of JSON Null object: %d", ATOMIC_INC(&jnull_cnt));
#endif
		return val;
	} else if (val == &JEMPTY_STR) {
#if COUNT_EMPTY_NULL
		PJ_LOG_REFCNT("attempt to grab ownership of empty string constant: %d", ATOMIC_INC(&jempty_cnt));
#endif
		return val;
	}

	val->m_refCnt++;
	TRACE_REF("inc refcnt to %d", val, val->m_refCnt);
	return val;
}

jvalue_ref jvalue_duplicate (jvalue_ref val)
{
	jvalue_ref result = val;
	SANITY_CHECK_POINTER(val);

	if (jis_null (val) || val == &JEMPTY_STR) return result;

	if (jis_object (val)) {
		result = jobject_create_hint (jobject_size (val));
		jobject_iter it;
		jobject_key_value pair;

		jobject_iter_init(&it, val);
		while (jobject_iter_next(&it, &pair))
		{
			jvalue_ref valueCopy = jvalue_duplicate (pair.value);
			if (!jobject_put (result, jvalue_copy (pair.key), valueCopy)) {
				j_release (&valueCopy);
				j_release (&pair.key);
				j_release (&result);
				result = NULL;
				break;
			}
		}
	} else if (jis_array (val)) {
		ssize_t arrSize = jarray_size (val);
		result = jarray_create_hint (NULL, arrSize);
		for (ssize_t i = arrSize - 1; i >= 0; i--) {
			if (!jarray_append (result, jvalue_duplicate (jarray_get (val, i)))) {
				j_release (&result);
				result = NULL;
				break;
			}
		}
		return result;
	} else {
		// string, number, & boolean are immutable, so no need to do an actual duplication
#if 0
		return jvalue_copy(val);
#else
		if (jis_string(val)) {
			result = jstring_create_copy(jstring_get_fast(val));
		} else if (jis_number(val)) {
			result = jnumber_duplicate(val);
		} else
			result = jboolean_create(jboolean_deref(val));
#endif
	}

	TRACE_REF("w/ refcnt of %d, deep copy to %p w/ refcnt of %d",
	          val, val->m_refCnt, result, result->m_refCnt);

	return result;
}

static void j_destroy_object (jvalue_ref obj) NON_NULL(1);
static void j_destroy_array (jvalue_ref arr) NON_NULL(1);
static void j_destroy_string (jvalue_ref str) NON_NULL(1);
static void j_destroy_number (jvalue_ref num) NON_NULL(1);
static inline void j_destroy_boolean (jvalue_ref boolean) NON_NULL(1);

void j_release (jvalue_ref *val)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER(val);
	if (UNLIKELY(*val == NULL)) {
		SANITY_KILL_POINTER(*val);
		return;
	}
	if (UNLIKELY(*val == &JNULL)) {
#if COUNT_EMPTY_NULL
		int newVal = ATOMIC_DEC(&jnull_cnt);
		PJ_LOG_REFCNT("attempt to release ownership of global JSON null object: %d", newVal);
		assert(newVal >= 0);
#endif
		SANITY_KILL_POINTER(*val);
		return;
	} else if (UNLIKELY(*val == &JEMPTY_STR)) {
#if COUNT_EMPTY_NULL
		int newVal = ATOMIC_DEC(&jempty_cnt);
		PJ_LOG_REFCNT("attempt to release ownership of global empty JSON string: %d", newVal);
		assert(newVal >= 0);
#endif
		SANITY_KILL_POINTER(*val);
		return;
	}

	assert((*val)->m_refCnt > 0);

	if ((*val)->m_refCnt == 1) {
		TRACE_REF("freeing because refcnt is 0: %s", *val, jvalue_tostring(*val, jschema_all()));
		if ((*val)->m_toStringDealloc) {
			PJ_LOG_MEM("Freeing string representation of jvalue %p", (*val)->m_toString);
			(*val)->m_toStringDealloc ((*val)->m_toString);
		}
		SANITY_KILL_POINTER((*val)->m_toString);

		switch ( (*val)->m_type) {
			case JV_OBJECT:
				j_destroy_object (*val);
				break;
			case JV_ARRAY:
				j_destroy_array (*val);
				break;
			case JV_STR:
				j_destroy_string (*val);
				break;
			case JV_NUM:
				j_destroy_number (*val);
				break;
			case JV_BOOL:
				j_destroy_boolean (*val);
				break;
			case JV_NULL:
				PJ_LOG_ERR("Invalid program state - should've already returned from j_release before this point");
				assert(false);
				break;
		}

		if ((*val)->m_backingBuffer.m_str) {
			if ((*val)->m_backingBufferMMap) {
				munmap((void *)(*val)->m_backingBuffer.m_str, (*val)->m_backingBuffer.m_len);
			} else {
				free((void *)(*val)->m_backingBuffer.m_str);
			}
		}

		SANITY_CLEAR_VAR((*val)->m_refCnt, 0);
		PJ_LOG_MEM("Freeing %p", *val);
		free (*val);
	} else if (UNLIKELY((*val)->m_refCnt < 0)) {
		PJ_LOG_ERR("reference counter messed up - memory corruption and/or random crashes are possible");
		assert(false);
	} else {
		(*val)->m_refCnt--;
		TRACE_REF("decrement ref cnt to %d: %s", *val, (*val)->m_refCnt, jvalue_tostring(*val, jschema_all()));
	}
	SANITY_KILL_POINTER(*val);
}

bool jis_null (jvalue_ref val)
{
	SANITY_CHECK_POINTER(val);
	if (val == &JNULL || val == NULL) {
		assert(val == NULL || val->m_type == JV_NULL);
	}
	else {
		assert(val->m_type != JV_NULL);
	}
	return val == &JNULL || val == NULL;
}

jvalue_ref jnull ()
{
	return &JNULL;
}

static const char *jvalue_tostring_internal_layer2 (jvalue_ref val, JSchemaInfoRef schemainfo, bool schemaNecessary)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER_RETURN_VALUE(val, "null");

	if (!val->m_toString) {
		bool parseok = false;
		StreamStatus error;
		JStreamRef generating = jstreamInternalWithInfo(schemainfo, TOP_None, schemaNecessary);
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

/************************* JSON OBJECT API **************************************/
#define DEREF_OBJ(ref) ((ref)->value.val_obj)

static unsigned long key_hash_raw (raw_buffer const *str) NON_NULL(1);
static unsigned long key_hash (jvalue_ref key) NON_NULL(1);

static unsigned long key_hash_raw (raw_buffer const *str)
{
	// djb2 algorithm
	unsigned long hash = 5381;
	char const *data = str->m_str;
	int count = str->m_len;
	int c = 0;

	assert(str->m_str != NULL);
	while (count--)
	{
		c = *data++;
		hash = ((hash << 5) + hash) + c;  // hash * 33 + c
	}
	return hash;
}

static bool check_insert_sanity(jvalue_ref parent, jvalue_ref child)
{
	// Sanity check that parent is object or array
	assert(jis_object(parent) || jis_array(parent));

	// Verify that the parent doesn't match the child
	if (UNLIKELY(child == parent)) {
		return false;
	}

	// Then check recursively child's children (if child is an array or an object)
	if (jis_array(child)) {
		for (int i = 0; i < jarray_size(child); i++) {
			jvalue_ref arr_elem = jarray_get(child, i);
			if(!check_insert_sanity(parent, arr_elem)) {
				return false;
			}
		}
	} else if (jis_object(child)) {
		jobject_iter it;
		jobject_key_value key_value;
		jobject_iter_init(&it, child);
		while (jobject_iter_next(&it, &key_value))
		{
			if(!check_insert_sanity(parent, key_value.value)) {
				return false;
			}
		}
	}

	return true;
}

static void j_destroy_object (jvalue_ref ref)
{
	g_hash_table_destroy(DEREF_OBJ(ref).m_members);
}

/* Has table key routines */
static guint _ObjKeyHash(gconstpointer key)
{
	jvalue_ref jkey = (jvalue_ref) key;
	return key_hash(jkey);
}

static gboolean _ObjKeyEqual(gconstpointer a, gconstpointer b)
{
	jvalue_ref ja = (jvalue_ref) a;
	jvalue_ref jb = (jvalue_ref) b;
	return jstring_equal_internal(ja, jb);
}

static void _ObjKeyValDestroy(gpointer data)
{
	jvalue_ref jdata = (jvalue_ref) data;
	j_release(&jdata);
}

jvalue_ref jobject_create ()
{
	jvalue_ref new_obj = jvalue_create (JV_OBJECT);
	CHECK_POINTER_RETURN_NULL(new_obj);
	DEREF_OBJ(new_obj).m_members = g_hash_table_new_full(_ObjKeyHash, _ObjKeyEqual,
	                                                     _ObjKeyValDestroy, _ObjKeyValDestroy);
	if (!DEREF_OBJ(new_obj).m_members)
	{
		free(new_obj);
		return NULL;
	}
	return new_obj;
}

//Helper function for jobject_to_string_append()
static bool key_value_to_string_append (jobject_key_value key_value, JStreamRef generating)
{
	//jvalue_to_string_append is enough for the key if schema validation isn't needed.
	//->o_key is called for validation.
	//jvalue_to_string_append (key_value.key, generating);

	if(generating->o_key(generating, key_value.key->value.val_str.m_data))
	{
		//Key was OK - now process the value.
		if(UNLIKELY(!jvalue_to_string_append (key_value.value, generating)))
		{
			return false;
		}
	}
	else
	{
		PJ_LOG_ERR("Schema validation error with key: '%s'", key_value.key->value.val_str.m_data.m_str);
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

jvalue_ref jobject_create_var (jobject_key_value item, ...)
{
	va_list ap;
	jobject_key_value arg;
	jvalue_ref new_object = jobject_create ();

	CHECK_POINTER_RETURN_NULL(new_object);

	if (item.key != NULL) {
		assert(jis_string(item.key));
		assert(item.value != NULL);

		if (UNLIKELY(!jobject_put (new_object, item.key, item.value))) {
			j_release (&new_object);
			return jnull();
		}

		va_start (ap, item);
		while ( (arg = va_arg (ap, jobject_key_value)).key != NULL) {
			assert(jis_string(arg.key));
			assert(arg.value != NULL);
			if (UNLIKELY(!jobject_put (new_object, arg.key, arg.value))) {
				PJ_LOG_ERR("Failed to insert requested key/value into new object");
				j_release (&new_object);
				new_object = jnull();
				break;
			}
		}
		va_end (ap);
	}

	return new_object;
}

jvalue_ref jobject_create_hint (int capacityHint)
{
	return jobject_create();
}

bool jis_object (jvalue_ref val)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER_RETURN_VALUE(val, false);
	assert_msg(s_inGdb || val->m_refCnt > 0, "%p is garbage", val);

	return val->m_type == JV_OBJECT;
}

size_t jobject_size(jvalue_ref obj)
{
	SANITY_CHECK_POINTER(obj);

	CHECK_CONDITION_RETURN_VALUE(!jis_object(obj), 0, "Attempt to retrieve size from something not an object %p", obj);

	if (!DEREF_OBJ(obj).m_members)
		return 0;
	return g_hash_table_size(DEREF_OBJ(obj).m_members);
}

bool jobject_get_exists (jvalue_ref obj, raw_buffer key, jvalue_ref *value)
{
	jvalue jkey =
	{
		.m_refCnt = 1,
		.m_type = JV_STR,
		.value = {
			.val_str = {
				.m_data = {
					.m_str = key.m_str,
					.m_len = key.m_len,
				},
			},
		},
	};

	return jobject_get_exists2(obj, &jkey, value);
}

bool jobject_get_exists2 (jvalue_ref obj, jvalue_ref key, jvalue_ref *value)
{
	jvalue_ref result;

	CHECK_CONDITION_RETURN_VALUE(jis_null(obj), false, "Attempt to cast null %p to object", obj);
	CHECK_CONDITION_RETURN_VALUE(!jis_object(obj), false, "Attempt to cast type %d to object (%d)", obj->m_type, JV_OBJECT);

	if (!DEREF_OBJ(obj).m_members)
		return false;

	result = g_hash_table_lookup(DEREF_OBJ(obj).m_members, key);
	if (!result)
		return false;

	if (value)
		*value = result;
	return true;
}

jvalue_ref jobject_get (jvalue_ref obj, raw_buffer key)
{
	jvalue_ref result = NULL;

	assert(key.m_str != NULL);
	if (jobject_get_exists (obj, key, &result))
		return result;
	return jnull ();
}

bool jobject_remove (jvalue_ref obj, raw_buffer key)
{
	SANITY_CHECK_POINTER(obj);

	CHECK_CONDITION_RETURN_VALUE(jis_null(obj), false, "Attempt to cast null %p to object", obj);
	CHECK_CONDITION_RETURN_VALUE(!jis_object(obj), false, "Attempt to cast type %d to object (%d)", obj->m_type, JV_OBJECT);

	if (!DEREF_OBJ(obj).m_members)
		return false;

	jvalue jkey =
	{
		.m_refCnt = 1,
		.m_type = JV_STR,
		.value = {
			.val_str = {
				.m_data = {
					.m_str = key.m_str,
					.m_len = key.m_len,
				},
			},
		},
	};

	return g_hash_table_remove(DEREF_OBJ(obj).m_members, &jkey);
}

bool jobject_set (jvalue_ref obj, raw_buffer key, jvalue_ref val)
{
	jvalue_ref newKey, newVal;

	if (!DEREF_OBJ(obj).m_members)
		return false;

	newVal = jvalue_copy (val);
	//CHECK_CONDITION_RETURN_VALUE(jis_null(newVal) && !jis_null(val), false, "Failed to create a copy of the value")

	newKey = jstring_create_copy (key);
	if (jis_null (newKey)) {
		PJ_LOG_ERR("Failed to create a copy of %.*s", (int)key.m_len, key.m_str);
		j_release (&newVal);
		return false;
	}

	if (UNLIKELY(!jobject_put(obj, newKey, newVal))) {
		j_release (&newKey);
		j_release (&newVal);
		return false;
	}
	return true;
}

bool jobject_set2(jvalue_ref obj, jvalue_ref key, jvalue_ref val)
{
	jvalue_ref new_key = jvalue_copy (key);
	if (UNLIKELY(!new_key))
	{
		PJ_LOG_ERR("Failed to create a copy of key %p", key);
		return false;
	}

	jvalue_ref new_val = jvalue_copy (val);
	if (UNLIKELY(!new_val))
	{
		PJ_LOG_ERR("Failed to create a copy of val %p", val);
		j_release(&new_key);
		return false;
	}

	if (UNLIKELY(!jobject_put(obj, new_key, new_val)))
	{
		j_release(&new_key);
		j_release(&new_val);
		return false;
	}
	return true;
}

bool jobject_put (jvalue_ref obj, jvalue_ref key, jvalue_ref val)
{
	SANITY_CHECK_POINTER(obj);
	SANITY_CHECK_POINTER(key);
	SANITY_CHECK_POINTER(val);

	assert(val != NULL);

	if (!DEREF_OBJ(obj).m_members)
		return false;

	CHECK_CONDITION_RETURN_VALUE(!jis_object(obj), false, "%p is %d not an object (%d)", obj, obj->m_type, JV_OBJECT);
	CHECK_POINTER_RETURN_NULL(key);
	CHECK_CONDITION_RETURN_VALUE(!jis_string(key), false, "%p is %d not a string (%d)", key, key->m_type, JV_STR);
	CHECK_CONDITION_RETURN_VALUE(jstring_size(key) == 0, false, "Object instance name is the empty string");

	if (val == NULL) {
		PJ_LOG_WARN("Please don't pass in NULL - use jnull() instead");
		val = jnull ();
	}

	if (!check_insert_sanity(obj, val)) {
		PJ_LOG_ERR("Error in object hierarchy. Inserting jvalue would create an illegal cyclic dependency");
		return false;
	}

	g_hash_table_replace(DEREF_OBJ(obj).m_members, key, val);
	return true;
}

// JSON Object iterators
bool jobject_iter_init(jobject_iter *iter, jvalue_ref obj)
{
	SANITY_CHECK_POINTER(obj);

	CHECK_CONDITION_RETURN_VALUE(!jis_object(obj), false, "Cannot iterate over non-object");
	CHECK_CONDITION_RETURN_VALUE(!DEREF_OBJ(obj).m_members, false, "The object isn't iterable");

	g_hash_table_iter_init(&iter->m_iter, DEREF_OBJ(obj).m_members);
	return true;
}

bool jobject_iter_next(jobject_iter *iter, jobject_key_value *keyval)
{
	return g_hash_table_iter_next(&iter->m_iter,
	                              (gpointer *)&keyval->key, (gpointer *)&keyval->value);
}

#undef DEREF_OBJ
/************************* JSON OBJECT API **************************************/

/************************* JSON ARRAY API  *************************************/
#define DEREF_ARR(ref) ((ref)->value.val_array)

static bool jarray_put_unsafe (jvalue_ref arr, ssize_t index, jvalue_ref val) NON_NULL(1, 3);
static inline ssize_t jarray_size_unsafe (jvalue_ref arr) NON_NULL(1);
static inline void jarray_size_increment_unsafe (jvalue_ref arr) NON_NULL(1);
static inline void jarray_size_decrement_unsafe (jvalue_ref arr) NON_NULL(1);
static jvalue_ref* jarray_get_unsafe (jvalue_ref arr, ssize_t index) NON_NULL(1);
static inline void jarray_size_set_unsafe (jvalue_ref arr, ssize_t newSize) NON_NULL(1);
static inline bool jarray_expand_capacity (jvalue_ref arr, ssize_t newSize) NON_NULL(1);
static bool jarray_expand_capacity_unsafe (jvalue_ref arr, ssize_t newSize) NON_NULL(1);
static void jarray_remove_unsafe (jvalue_ref arr, ssize_t index) NON_NULL(1);

static bool valid_index_bounded (jvalue_ref arr, ssize_t index) NON_NULL(1);
static bool valid_index_bounded (jvalue_ref arr, ssize_t index)
{
	SANITY_CHECK_POINTER(arr);
	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), false, "Trying to test index bounds on non-array %p", arr);
	CHECK_CONDITION_RETURN_VALUE(index < 0, false, "Negative array index %zd", index);

	CHECK_CONDITION_RETURN_VALUE(index >= jarray_size(arr), false, "Index %zd out of bounds of array size %zd", index, jarray_size(arr));

	return true;
}

static void j_destroy_array (jvalue_ref arr)
{
	SANITY_CHECK_POINTER(arr);
	SANITY_CHECK_POINTER(DEREF_ARR(arr).m_bigBucket);
	assert(jis_array(arr));

#ifdef DEBUG_FREED_POINTERS
	for (ssize_t i = jarray_size(arr); i < DEREF_ARR(arr).m_capacity; i++) {
		jvalue_ref *outsideValue = jarray_get_unsafe(arr, i);
		assert(*outsideValue == NULL || *outsideValue == FREED_POINTER);
	}
#endif

	assert(jarray_size(arr) >= 0);

	for (int i = jarray_size(arr) - 1; i >= 0; i--)
		jarray_remove_unsafe(arr, i);

	assert(jarray_size(arr) == 0);

	PJ_LOG_MEM("Destroying array bucket at %p", DEREF_ARR(arr).m_bigBucket);
	SANITY_FREE(free, jvalue_ref *, DEREF_ARR(arr).m_bigBucket, DEREF_ARR(arr).m_capacity - ARRAY_BUCKET_SIZE);
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

jvalue_ref jarray_create (jarray_opts opts)
{
	jvalue_ref new_array = jvalue_create (JV_ARRAY);
	CHECK_ALLOC_RETURN_NULL(new_array);

	DEREF_ARR(new_array).m_capacity = ARRAY_BUCKET_SIZE;
	return new_array;
}

jvalue_ref jarray_create_var (jarray_opts opts, ...)
{
	// jarray_create_hint will take care of the capacity for us
	jvalue_ref new_array = jarray_create_hint (opts, 1);
	jvalue_ref element;

	CHECK_POINTER_RETURN_NULL(new_array);

	va_list iter;

	va_start (iter, opts);
	while ( (element = va_arg (iter, jvalue_ref)) != NULL) {
		jarray_put_unsafe (new_array, jarray_size_unsafe (new_array), element);
	}
	va_end (iter);

	return new_array;
}

/**
 * Create an empty array with the specified properties and the hint that the array will eventually contain capacityHint elements.
 *
 * @param opts The options for the array (currently unspecified).  NULL indicates use default options.
 * @param capacityHint A guess-timate of the eventual size of the array (implementation is free to ignore this).
 * @return A reference to the created array value.  The caller has ownership.
 */
jvalue_ref jarray_create_hint (jarray_opts opts, size_t capacityHint)
{
	jvalue_ref new_array = jarray_create (opts);
	if (UNLIKELY(capacityHint == 0)) {
		PJ_LOG_INFO("Non-recommended use of API providing a hint of 0 elements.  instead, maybe use jarray_create?");
	} else if (LIKELY(new_array != NULL)) {
		jarray_expand_capacity_unsafe (new_array, capacityHint);
	}

	return new_array;
}

/**
 * Determine whether or not the reference to the JSON value represents an array.
 *
 * @param val The reference to test
 * @return True if it is an array, false otherwise.
 */
bool jis_array (jvalue_ref val)
{
	SANITY_CHECK_POINTER(val);
	CHECK_POINTER_RETURN_VALUE(val, false);
	assert(s_inGdb || val->m_refCnt > 0);

	return val->m_type == JV_ARRAY;
}

ssize_t jarray_size (jvalue_ref arr)
{
	SANITY_CHECK_POINTER(arr);
	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), 0, "Attempt to get array size of non-array %p", arr);
	return jarray_size_unsafe (arr);
}

static inline ssize_t jarray_size_unsafe (jvalue_ref arr)
{
	assert(jis_array(arr));

	return DEREF_ARR(arr).m_size;
}

static inline void jarray_size_increment_unsafe (jvalue_ref arr)
{
	assert(jis_array(arr));

	++DEREF_ARR(arr).m_size;

	assert(jarray_size_unsafe(arr) <= DEREF_ARR(arr).m_capacity);
}

static inline void jarray_size_decrement_unsafe (jvalue_ref arr)
{
	assert(jis_array(arr));

	--DEREF_ARR(arr).m_size;

	assert(jarray_size_unsafe(arr) >= 0);
}

static inline void jarray_size_set_unsafe (jvalue_ref arr, ssize_t newSize)
{
	assert(jis_array(arr));
	assert(newSize <= DEREF_ARR(arr).m_capacity);

	DEREF_ARR(arr).m_size = newSize;
}

static jvalue_ref* jarray_get_unsafe (jvalue_ref arr, ssize_t index)
{
	assert(jis_array(arr));
	assert(index >= 0);
	assert(index < DEREF_ARR(arr).m_capacity);

	if (UNLIKELY(OUTSIDE_ARR_BUCKET_RANGE(index))) {
		return &DEREF_ARR(arr).m_bigBucket [index - ARRAY_BUCKET_SIZE];
	}
	assert(index < ARRAY_BUCKET_SIZE);
	return &DEREF_ARR(arr).m_smallBucket [index];
}

jvalue_ref jarray_get (jvalue_ref arr, ssize_t index)
{
	jvalue_ref result;

	CHECK_CONDITION_RETURN_VALUE(!valid_index_bounded(arr, index), jnull(), "Attempt to get array element from %p with out-of-bounds index value %zd", arr, index);

	result = * (jarray_get_unsafe (arr, index));
	if (result == NULL)
	// need to fix up in case we haven't assigned anything to that space - it's initialized to NULL (JSON undefined)
	result = jnull ();
	return result;
}

static void jarray_remove_unsafe (jvalue_ref arr, ssize_t index)
{
	ssize_t i;
	jvalue_ref *hole, *toMove;
	ssize_t array_size;

	assert(valid_index_bounded(arr, index));

	hole = jarray_get_unsafe (arr, index);
	assert (hole != NULL);
	j_release (hole);

	array_size = jarray_size_unsafe (arr);

	// Shift down all elements
	for (i = index + 1; i < array_size; i++) {
		toMove = jarray_get_unsafe (arr, i);
		*hole = *toMove;
		hole = toMove;
	}

	jarray_size_decrement_unsafe (arr);

	// This is necessary because someone else might reference this position, and
	// they need to know that it's empty (in case they need to free it).
	*hole = NULL;
}

bool jarray_remove (jvalue_ref arr, ssize_t index)
{
	CHECK_CONDITION_RETURN_VALUE(!valid_index_bounded(arr, index), jnull(), "Attempt to get array element from %p with out-of-bounds index value %zd", arr, index);

	jarray_remove_unsafe (arr, index);

	return true;
}

static inline bool jarray_expand_capacity (jvalue_ref arr, ssize_t newSize)
{
	assert(jis_array(arr));

	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), false, "Attempt to expand something that wasn't a JSON array reference: %p", arr);

	return jarray_expand_capacity_unsafe (arr, newSize);
}

static bool jarray_expand_capacity_unsafe (jvalue_ref arr, ssize_t newSize)
{
	assert(jis_array(arr));
	assert(newSize >= 0);

	if (newSize > DEREF_ARR(arr).m_capacity) {
		// m_capacity is always a minimum of the bucket size
		assert(OUTSIDE_ARR_BUCKET_RANGE(newSize));
		assert(newSize > ARRAY_BUCKET_SIZE);
		jvalue_ref *newBigBucket = realloc (DEREF_ARR(arr).m_bigBucket, sizeof(jvalue_ref) * (newSize - ARRAY_BUCKET_SIZE));
		if (UNLIKELY(newBigBucket == NULL)) {
			assert(false);
			return false;
		}

		PJ_LOG_MEM("Resized %p from %zu bytes to %p with %zu bytes", DEREF_ARR(arr).m_bigBucket, sizeof(jvalue_ref)*(DEREF_ARR(arr).m_capacity - ARRAY_BUCKET_SIZE), newBigBucket, sizeof(jvalue_ref)*(newSize - ARRAY_BUCKET_SIZE));

		for (ssize_t x = DEREF_ARR(arr).m_capacity - ARRAY_BUCKET_SIZE; x < newSize - ARRAY_BUCKET_SIZE; x++)
			newBigBucket[x] = NULL;

		DEREF_ARR(arr).m_bigBucket = newBigBucket;
		DEREF_ARR(arr).m_capacity = newSize;
	}

	return true;
}

static bool jarray_put_unsafe (jvalue_ref arr, ssize_t index, jvalue_ref val)
{
	jvalue_ref *old;
	SANITY_CHECK_POINTER(arr);
	assert(jis_array(arr));

	if (!check_insert_sanity(arr, val)) {
		PJ_LOG_ERR("Error in object hierarchy. Inserting jvalue would create an illegal cyclic dependency");
		return false;
	}

	if (!jarray_expand_capacity_unsafe (arr, index + 1)) {
		PJ_LOG_WARN("Failed to expand array to allocate element - memory allocation problem?");
		return false;
	}

	old = jarray_get_unsafe(arr, index);
	j_release(old);
	*old = val;

	if (index >= jarray_size_unsafe (arr)) jarray_size_set_unsafe (arr, index + 1);

	return true;
}

bool jarray_set (jvalue_ref arr, ssize_t index, jvalue_ref val)
{
	jvalue_ref arr_val;

	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), false, "Attempt to get array size of non-array %p", arr);
	CHECK_CONDITION_RETURN_VALUE(index < 0, false, "Attempt to set array element for %p with negative index value %zd", arr, index);

	if (UNLIKELY(val == NULL)) {
		PJ_LOG_WARN("incorrect API use - please pass an actual reference to a JSON null if that's what you want - assuming that's what you meant");
		val = jnull ();
	}

	arr_val = jvalue_copy (val);
	CHECK_ALLOC_RETURN_VALUE(arr_val, false);

	return jarray_put_unsafe (arr, index, arr_val);
}

bool jarray_put (jvalue_ref arr, ssize_t index, jvalue_ref val)
{
	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), false, "Attempt to insert into non-array %p", arr);
	CHECK_CONDITION_RETURN_VALUE(index < 0, false, "Attempt to insert array element for %p with negative index value %zd", arr, index);

	if (UNLIKELY(val == NULL)) {
		PJ_LOG_WARN("incorrect API use - please pass an actual reference to a JSON null if that's what you want - assuming that's the case");
		val = jnull ();
	}

	return jarray_put_unsafe (arr, index, val);
}

bool jarray_append (jvalue_ref arr, jvalue_ref val)
{
	SANITY_CHECK_POINTER(val);
	SANITY_CHECK_POINTER(arr);

	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), false, "Attempt to append into non-array %p", arr);

	if (UNLIKELY(val == NULL)) {
		PJ_LOG_WARN("incorrect API use - please pass an actual reference to a JSON null if that's what you want - assuming that's the case");
		val = jnull ();
	}

	return jarray_put_unsafe (arr, jarray_size_unsafe (arr), val);
}

/**
 * Insert the value into the array before the specified position.
 *
 * arr[index] now contains val, with all elements shifted appropriately.
 *
 * NOTE: It is unspecified behaviour to modify val after passing it to this array
 *
 * @param arr
 * @param index
 * @param val
 *
 * @see jarray_append
 * @see jarray_put
 */
bool jarray_insert(jvalue_ref arr, ssize_t index, jvalue_ref val)
{
	ssize_t j;

	SANITY_CHECK_POINTER(arr);

	CHECK_CONDITION_RETURN_VALUE(!jis_array(arr), false, "Array to insert into isn't a valid reference to a JSON DOM node: %p", arr);
	CHECK_CONDITION_RETURN_VALUE(index < 0, false, "Invalid index - must be >= 0: %zd", index);

	if (!check_insert_sanity(arr, val)) {
		PJ_LOG_ERR("Error in object hierarchy. Inserting jvalue would create an illegal cyclic dependency");
		return false;
	}

	{
		jvalue_ref *toMove, *hole;
		// we increment the size of the array
		jarray_put_unsafe(arr, jarray_size_unsafe(arr), jnull());

		// stopping at the first jis_null as an optimization is actually
		// wrong because we change the array structure.  we have to move up
		// all the elements.
		hole = jarray_get_unsafe(arr, jarray_size_unsafe(arr) - 1);

		for (j = jarray_size_unsafe(arr) - 1; j > index; j--, hole = toMove) {
			toMove = jarray_get_unsafe(arr, j - 1);
			*hole = *toMove;
		}

		*hole = val;
	}

	return true;
}

// Helper function to check insert sanity for jarray_splice
static bool jarray_splice_check_insert_sanity(jvalue_ref arr, jvalue_ref arr2)
{
	assert(jis_array(arr));
	assert(jis_array(arr2));

	for (int i = 0; i < jarray_size(arr2); i++) {
		jvalue_ref arr_elem = jarray_get(arr2, i);
		if (!check_insert_sanity(arr, arr_elem)) {
			return false;
		}
	}

	return true;
}

bool jarray_splice (jvalue_ref array, ssize_t index, ssize_t toRemove, jvalue_ref array2, ssize_t begin, ssize_t end, JSpliceOwnership ownership)
{
	ssize_t i, j;
	size_t removable = toRemove;
	jvalue_ref *valueInOtherArray;
	jvalue_ref valueToInsert;

	if (LIKELY(removable)) {
		CHECK_CONDITION_RETURN_VALUE(!valid_index_bounded(array, index), false, "Splice index is invalid");
		CHECK_CONDITION_RETURN_VALUE(!valid_index_bounded(array, index + toRemove - 1), false, "To remove amount is out of bounds of array");
	} else {
		SANITY_CHECK_POINTER(array);
		CHECK_CONDITION_RETURN_VALUE(!jis_array(array), false, "Array isn't valid %p", array);
		if (index < 0) index = 0;
	}
	CHECK_CONDITION_RETURN_VALUE(begin >= end, false, "Invalid range to copy from second array: [%zd, %zd)", begin, end); // set notation
	CHECK_CONDITION_RETURN_VALUE(!valid_index_bounded(array2, begin), false, "Start index is invalid for second array");
	CHECK_CONDITION_RETURN_VALUE(!valid_index_bounded(array2, end - 1), false, "End index is invalid for second array");
	CHECK_CONDITION_RETURN_VALUE(toRemove < 0, false, "Invalid amount %zd to remove during splice", toRemove);

	if (!jarray_splice_check_insert_sanity(array, array2)) {
		PJ_LOG_ERR("Error in object hierarchy. Splicing array would create an illegal cyclic dependency");
		return false;
	}

	for (i = index, j = begin; removable && j < end; i++, removable--, j++) {
		assert(valid_index_bounded(array, i));
		assert(valid_index_bounded(array2, j));
		valueInOtherArray = jarray_get_unsafe(array2, j);
		valueToInsert = *valueInOtherArray;
		assert(valueInOtherArray != NULL);
		switch (ownership) {
			case SPLICE_TRANSFER:
				*valueInOtherArray = NULL;
				jarray_size_decrement_unsafe (array2);
				break;
			case SPLICE_NOCHANGE:
				break;
			case SPLICE_COPY:
				valueToInsert = jvalue_copy(valueToInsert);
				break;
		}
		jarray_put_unsafe (array, i, valueToInsert);
	}

	if (removable != 0) {
		assert (j == end);
		assert (toRemove > end - begin);

		while (removable) {
			jarray_remove_unsafe (array, i);
			removable--;
		}
	} else if (j < end) {
		assert (toRemove < end - begin);
		assert (removable == 0);

		jarray_expand_capacity_unsafe (array, jarray_size_unsafe (array) + (end - j));

		// insert any remaining elements that don't overlap the amount to remove
		for (; j < end; j++, i++) {
			assert(valid_index_bounded(array2, j));

			valueInOtherArray = jarray_get_unsafe(array2, j);
			valueToInsert = *valueInOtherArray;
			assert(valueInOtherArray != NULL);
			switch (ownership) {
				case SPLICE_TRANSFER:
					*valueInOtherArray = NULL;
					jarray_size_decrement_unsafe (array2);
					break;
				case SPLICE_NOCHANGE:
					break;
				case SPLICE_COPY:
					valueToInsert = jvalue_copy(valueToInsert);
					break;
			}
			if (UNLIKELY(!jarray_insert(array, i, valueToInsert))) {
				PJ_LOG_ERR("How did this happen? Failed to insert %zd from second array into %zd of first array", j, i);
				return false;
			}
		}
	} else {
		assert (toRemove == end - begin);
	}
	return true;
}

bool jarray_splice_inject (jvalue_ref array, ssize_t index, jvalue_ref arrayToInject, JSpliceOwnership ownership)
{
	return jarray_splice (array, index, 0, arrayToInject, 0, jarray_size (arrayToInject), ownership);
}

bool jarray_splice_append (jvalue_ref array, jvalue_ref arrayToAppend, JSpliceOwnership ownership)
{
	return jarray_splice (array, jarray_size (array) - 1, 0, arrayToAppend, 0, jarray_size (arrayToAppend), ownership);
}

#undef DEREF_ARR

/****************************** JSON STRING API ************************/
#define DEREF_STR(ref) ((ref)->value.val_str)

#define SANITY_CHECK_JSTR_BUFFER(jval)					\
	do {								\
		SANITY_CHECK_POINTER(jval);				\
		SANITY_CHECK_POINTER(DEREF_STR(jval).m_data.m_str);	\
		SANITY_CHECK_MEMORY(DEREF_STR(jval).m_data.m_str, DEREF_STR(jval).m_data.m_len);	\
		SANITY_CHECK_POINTER(DEREF_STR(jval).m_dealloc);	\
	} while (0)

static inline bool jstring_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	bool result = (generating->string (generating, DEREF_STR(jref).m_data) != NULL);
	if (UNLIKELY(!result)) {
		PJ_LOG_ERR("Schema validation error, string '%s' did not validate against schema", DEREF_STR(jref).m_data.m_str);
	}
	return result;
}

static void j_destroy_string (jvalue_ref str)
{
	assert(str != &JEMPTY_STR);
	SANITY_CHECK_JSTR_BUFFER(str);
#ifdef _DEBUG
	if (str == NULL) {
		PJ_LOG_ERR("Internal error - string reference to release the string buffer for is NULL");
		return;
	}
#endif
	if (DEREF_STR(str).m_dealloc) {
		PJ_LOG_MEM("Destroying string %p", DEREF_STR(str).m_data.m_str);
		SANITY_CLEAR_MEMORY(DEREF_STR(str).m_data.m_str, DEREF_STR(str).m_data.m_len);
		SANITY_FREE_CUST(DEREF_STR(str).m_dealloc, char *, DEREF_STR(str).m_data.m_str, DEREF_STR(str).m_data.m_len);
	}
	PJ_LOG_MEM("Changing string %p to NULL for %p", DEREF_STR(str).m_data.m_str, str);
	SANITY_KILL_POINTER(DEREF_STR(str).m_data.m_str);
	SANITY_CLEAR_VAR(DEREF_STR(str).m_data.m_len, -1);
}

static unsigned long key_hash (jvalue_ref key)
{
	assert(jis_string(key));
	return key_hash_raw (&DEREF_STR(key).m_data);
}

jvalue_ref jstring_empty ()
{
	return &JEMPTY_STR;
}

jvalue_ref jstring_create (const char *cstring)
{
	return jstring_create_utf8 (cstring, strlen (cstring));
}

jvalue_ref jstring_create_utf8 (const char *cstring, ssize_t length)
{
	if (length < 0) length = strlen (cstring);
	return jstring_create_copy (j_str_to_buffer (cstring, length));
}

jvalue_ref jstring_create_copy (raw_buffer str)
{
	char *copyBuffer;
	copyBuffer = calloc (str.m_len + SAFE_TERM_NULL_LEN, sizeof(char));
	if (copyBuffer == NULL) {
		PJ_LOG_ERR("Failed to allocate space for private string copy");
		return jnull();
	}
	memcpy(copyBuffer, str.m_str, str.m_len);

	jvalue_ref new_str = jstring_create_nocopy_full(j_str_to_buffer(copyBuffer, str.m_len), free);
	CHECK_POINTER_RETURN_NULL(new_str);

	DEREF_STR(new_str).m_data.m_len = str.m_len;
	SANITY_CHECK_JSTR_BUFFER(new_str);

	return new_str;
}

bool jis_string (jvalue_ref str)
{
#ifdef DEBUG_FREED_POINTERS
	if (str->m_type == JV_STR)
		SANITY_CHECK_JSTR_BUFFER(str);
#endif
	CHECK_POINTER_RETURN_VALUE(str, false);
	assert(s_inGdb || str->m_refCnt > 0);

	return str->m_type == JV_STR;
}

jvalue_ref jstring_create_nocopy (raw_buffer val)
{
	return jstring_create_nocopy_full (val, NULL);
}

jvalue_ref jstring_create_nocopy_full (raw_buffer val, jdeallocator buffer_dealloc)
{
	jvalue_ref new_string;

	SANITY_CHECK_POINTER(val.m_str);
	SANITY_CHECK_MEMORY(val.m_str, val.m_len);
	CHECK_CONDITION_RETURN_VALUE(val.m_str == NULL, jnull(), "Invalid string to set JSON string to NULL");
	if (val.m_len == 0) {
		if (buffer_dealloc) buffer_dealloc((void *)val.m_str);
		return &JEMPTY_STR;
	}

	new_string = jvalue_create (JV_STR);
	CHECK_POINTER_RETURN_NULL(new_string);

	DEREF_STR(new_string).m_dealloc = buffer_dealloc;
	DEREF_STR(new_string).m_data = val;

	SANITY_CHECK_JSTR_BUFFER(new_string);

	return new_string;
}

ssize_t jstring_size (jvalue_ref str)
{
	SANITY_CHECK_JSTR_BUFFER(str);
	CHECK_CONDITION_RETURN_VALUE(!jis_string(str), 0, "Invalid parameter - %d is not a string (%d)", str->m_type, JV_STR);

	assert(DEREF_STR(str).m_data.m_str);

	return DEREF_STR(str).m_data.m_len;
}

raw_buffer jstring_get (jvalue_ref str)
{
	SANITY_CHECK_JSTR_BUFFER(str);
	char *str_copy;

	// performs the error checking for us as well
	raw_buffer raw_str = jstring_get_fast (str);
	if (UNLIKELY(raw_str.m_str == NULL)) return raw_str;

	str_copy = calloc (raw_str.m_len + 1, sizeof(char));
	if (str_copy == NULL) {
		return j_str_to_buffer (NULL, 0);
	}

	memcpy (str_copy, raw_str.m_str, raw_str.m_len);

	return j_str_to_buffer (str_copy, raw_str.m_len);
}

raw_buffer jstring_get_fast (jvalue_ref str)
{
	SANITY_CHECK_JSTR_BUFFER(str);
	CHECK_CONDITION_RETURN_VALUE(!jis_string(str), j_str_to_buffer(NULL, 0), "Invalid API use - attempting to get string buffer for non JSON string %p", str);

	return DEREF_STR(str).m_data;
}

static bool jstring_equal_internal(jvalue_ref str, jvalue_ref other)
{
	SANITY_CHECK_JSTR_BUFFER(str);
	SANITY_CHECK_JSTR_BUFFER(other);
	return str == other ||
			jstring_equal_internal2(str, &DEREF_STR(other).m_data);
}

static inline bool jstring_equal_internal2(jvalue_ref str, raw_buffer *other)
{
	SANITY_CHECK_JSTR_BUFFER(str);
	SANITY_CHECK_MEMORY(other->m_str, other->m_len);
	return jstring_equal_internal3(&DEREF_STR(str).m_data, other);
}

static bool jstring_equal_internal3(raw_buffer *str, raw_buffer *other)
{
	SANITY_CHECK_MEMORY(str->m_str, str->m_len);
	SANITY_CHECK_MEMORY(other->m_str, other->m_len);
	return str->m_str == other->m_str ||
			(
					str->m_len == other->m_len &&
					memcmp(str->m_str, other->m_str, str->m_len) == 0
			);
}

bool jstring_equal (jvalue_ref str, jvalue_ref other)
{
	SANITY_CHECK_JSTR_BUFFER(str);
	SANITY_CHECK_JSTR_BUFFER(other);

	if (UNLIKELY(!jis_string(str) || !jis_string(other))) {
		PJ_LOG_WARN("attempting to check string equality but not using a JSON string");
		return false;
	}

	return jstring_equal_internal(str, other);
}

bool jstring_equal2 (jvalue_ref str, raw_buffer other)
{
	if (UNLIKELY(!jis_string(str))) {
		PJ_LOG_WARN("attempting to check string equality but not a JSON string");
		return false;
	}

	return jstring_equal_internal2(str, &other);
}

#undef DEREF_STR
/******************************** JSON STRING API **************************************/
#define DEREF_NUM(ref) ((ref)->value.val_num)

static void j_destroy_number (jvalue_ref num)
{
	SANITY_CHECK_POINTER(num);
	assert(jis_number(num));

	if (DEREF_NUM(num).m_type != NUM_RAW) {
		return;
	}

	assert(DEREF_NUM(num).value.raw.m_str != NULL);
	SANITY_CHECK_POINTER(DEREF_NUM(num).value.raw.m_str);

	if (DEREF_NUM(num).m_rawDealloc) {
		PJ_LOG_MEM("Destroying raw numeric string %p", DEREF_NUM(num).value.raw.m_str);
		DEREF_NUM(num).m_rawDealloc ((char *)DEREF_NUM(num).value.raw.m_str);
	}
	PJ_LOG_MEM("Clearing raw numeric string from %p to NULL for %p", DEREF_NUM(num).value.raw.m_str, num);
	SANITY_KILL_POINTER(DEREF_NUM(num).value.raw.m_str);
	SANITY_CLEAR_VAR(DEREF_NUM(num).value.raw.m_len, 0);
}

static bool jnumber_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	SANITY_CHECK_POINTER(jref);
	if (DEREF_NUM(jref).m_error) {
		PJ_LOG_WARN("converting a number that has an error (%d) set to a string", DEREF_NUM(jref).m_error);
	}

	bool ok = false;
	switch (DEREF_NUM(jref).m_type) {
		case NUM_RAW:
			assert(DEREF_NUM(jref).value.raw.m_len != 0);
			ok = (generating->number (generating, DEREF_NUM(jref).value.raw) != NULL);
			break;
		case NUM_FLOAT:
			ok = (generating->floating (generating, DEREF_NUM(jref).value.floating) != NULL);
			break;
		case NUM_INT:
			ok = (generating->integer (generating, DEREF_NUM(jref).value.integer) != NULL);
			break;
		default:
			// mismatched on purpose so that generation yields an error
			assert(false);
			generating->o_begin (generating);
			raw_buffer asStrBuf = J_CSTR_TO_BUF("Error - Unrecognized number type");
			generating->string (generating, asStrBuf);
			generating->integer (generating, jref->value.val_num.m_type);
			break;
	}

	return ok;
}

jvalue_ref jnumber_duplicate (jvalue_ref num)
{
	assert (jis_number(num));

	switch (DEREF_NUM(num).m_type) {
	case NUM_RAW:
		return jnumber_create(DEREF_NUM(num).value.raw);
	case NUM_FLOAT:
		return jnumber_create_f64(DEREF_NUM(num).value.floating);
	case NUM_INT:
		return jnumber_create_i64(DEREF_NUM(num).value.integer);
	}
	assert(false);
	return jnull();
}

jvalue_ref jnumber_create (raw_buffer str)
{
	char *createdBuffer = NULL;
	jvalue_ref new_number;

	assert(str.m_str != NULL);
	assert(str.m_len > 0);

	CHECK_POINTER_RETURN_VALUE(str.m_str, jnull());
	CHECK_CONDITION_RETURN_VALUE(str.m_len <= 0, jnull(), "Invalid length parameter for numeric string %s", str.m_str);

	createdBuffer = (char *) calloc (str.m_len + NUM_TERM_NULL, sizeof(char));
	CHECK_ALLOC_RETURN_VALUE(createdBuffer, jnull());

	memcpy (createdBuffer, str.m_str, str.m_len);
	str.m_str = createdBuffer;
	new_number = jnumber_create_unsafe(str, free);
	if (jis_null(new_number))
		free(createdBuffer);

	return new_number;
}

jvalue_ref jnumber_create_unsafe (raw_buffer str, jdeallocator strFree)
{
	jvalue_ref new_number;

	assert(str.m_str != NULL);
	assert(str.m_len > 0);

	CHECK_POINTER_RETURN_VALUE(str.m_str, jnull());
	CHECK_CONDITION_RETURN_VALUE(str.m_len == 0, jnull(), "Invalid length parameter for numeric string %s", str.m_str);

	new_number = jvalue_create (JV_NUM);
	CHECK_ALLOC_RETURN_NULL(new_number);

	DEREF_NUM(new_number).m_type = NUM_RAW;
	DEREF_NUM(new_number).value.raw = str;
	DEREF_NUM(new_number).m_rawDealloc = strFree;

	return new_number;
}

jvalue_ref jnumber_create_f64 (double number)
{
	jvalue_ref new_number;

	CHECK_CONDITION_RETURN_VALUE(isnan(number), jnull(), "NaN has no representation in JSON");
	CHECK_CONDITION_RETURN_VALUE(isinf(number), jnull(), "Infinity has no representation in JSON");

	new_number = jvalue_create (JV_NUM);
	CHECK_ALLOC_RETURN_NULL(new_number);

	DEREF_NUM(new_number).m_type = NUM_FLOAT;
	DEREF_NUM(new_number).value.floating = number;

	return new_number;
}

jvalue_ref jnumber_create_i32 (int32_t number)
{
	return jnumber_create_i64 (number);
}

jvalue_ref jnumber_create_i64 (int64_t number)
{
	jvalue_ref new_number;

	new_number = jvalue_create (JV_NUM);
	CHECK_ALLOC_RETURN_NULL(new_number);

	DEREF_NUM(new_number).m_type = NUM_INT;
	DEREF_NUM(new_number).value.integer = number;

	return new_number;
}

jvalue_ref jnumber_create_converted(raw_buffer raw)
{
	jvalue_ref new_number;

	new_number = jvalue_create(JV_NUM);
	CHECK_ALLOC_RETURN_NULL(new_number);

	if (CONV_OK != jstr_to_i64(&raw, &DEREF_NUM(new_number).value.integer)) {
		DEREF_NUM(new_number).m_error = jstr_to_double(&raw, &DEREF_NUM(new_number).value.floating);
		if (DEREF_NUM(new_number).m_error != CONV_OK) {
			PJ_LOG_ERR("Number '%.*s' doesn't convert perfectly to a native type",
					(int)raw.m_len, raw.m_str);
			assert(false);
		}
	}

	return new_number;
}

int jnumber_compare(jvalue_ref number, jvalue_ref toCompare)
{
	SANITY_CHECK_POINTER(number);
	SANITY_CHECK_POINTER(toCompare);

	assert(jis_number(number));
	assert(jis_number(toCompare));

	switch (DEREF_NUM(toCompare).m_type) {
		case NUM_FLOAT:
			return jnumber_compare_f64(number, DEREF_NUM(toCompare).value.floating);
		case NUM_INT:
			return jnumber_compare_i64(number, DEREF_NUM(toCompare).value.integer);
		case NUM_RAW:
		{
			int64_t asInt;
			double asFloat;
			if (CONV_OK == jstr_to_i64(&DEREF_NUM(toCompare).value.raw, &asInt))
				return jnumber_compare_i64(number, asInt);
			if (CONV_OK != jstr_to_double(&DEREF_NUM(toCompare).value.raw, &asFloat)) {
				PJ_LOG_ERR("Comparing against something that can't be represented as a float: '%.*s'",
						(int)DEREF_NUM(toCompare).value.raw.m_len, DEREF_NUM(toCompare).value.raw.m_str);
			}
			return jnumber_compare_f64(number, asFloat);
		}
		default:
			PJ_LOG_ERR("Unknown type for toCompare - corruption?");
			assert(false);
			return -50;
	}
}

int jnumber_compare_i64(jvalue_ref number, int64_t toCompare)
{
	SANITY_CHECK_POINTER(number);
	assert(jis_number(number));

	switch (DEREF_NUM(number).m_type) {
		case NUM_FLOAT:
			return DEREF_NUM(number).value.floating > toCompare ? 1 :
				(DEREF_NUM(number).value.floating < toCompare ? -1 : 0);
		case NUM_INT:
			return DEREF_NUM(number).value.integer > toCompare ? 1 :
				(DEREF_NUM(number).value.integer < toCompare ? -1 : 0);
		case NUM_RAW:
		{
			int64_t asInt;
			if (CONV_OK == jstr_to_i64(&DEREF_NUM(number).value.raw, &asInt)) {
				return asInt > toCompare ? 1 :
						(asInt < toCompare ? -1 : 0);
			}
			double asFloat;
			if (CONV_OK != jstr_to_double(&DEREF_NUM(number).value.raw, &asFloat)) {
				PJ_LOG_ERR("Comparing '%"PRId64 "' against something that can't be represented as a float: '%.*s'",
						toCompare, (int)DEREF_NUM(number).value.raw.m_len, DEREF_NUM(number).value.raw.m_str);
			}
			return asFloat > toCompare ? 1 : (asFloat < toCompare ? -1 : 0);
		}
		default:
			PJ_LOG_ERR("Unknown type - corruption?");
			assert(false);
			return -50;
	}
}

int jnumber_compare_f64(jvalue_ref number, double toCompare)
{
	SANITY_CHECK_POINTER(number);
	assert(jis_number(number));

	switch (DEREF_NUM(number).m_type) {
		case NUM_FLOAT:
			return DEREF_NUM(number).value.floating > toCompare ? 1 :
				(DEREF_NUM(number).value.floating < toCompare ? -1 : 0);
		case NUM_INT:
			return DEREF_NUM(number).value.integer > toCompare ? 1 :
				(DEREF_NUM(number).value.integer < toCompare ? -1 : 0);
		case NUM_RAW:
		{
			int64_t asInt;
			if (CONV_OK == jstr_to_i64(&DEREF_NUM(number).value.raw, &asInt)) {
				return asInt > toCompare ? 1 :
						(asInt < toCompare ? -1 : 0);
			}
			double asFloat;
			if (CONV_OK != jstr_to_double(&DEREF_NUM(number).value.raw, &asFloat)) {
				PJ_LOG_ERR("Comparing '%lf' against something that can't be represented as a float: '%.*s'",
						toCompare, (int)DEREF_NUM(number).value.raw.m_len, DEREF_NUM(number).value.raw.m_str);
			}
			return asFloat > toCompare ? 1 : (asFloat < toCompare ? -1 : 0);
		}
		default:
			PJ_LOG_ERR("Unknown type - corruption?");
			assert(false);
			return -50;
	}
}

bool jnumber_has_error (jvalue_ref number)
{
	return DEREF_NUM(number).m_error != CONV_OK;
}

bool jis_number (jvalue_ref num)
{
	SANITY_CHECK_POINTER(num);
	CHECK_POINTER_RETURN_VALUE(num, false);
	assert(s_inGdb || num->m_refCnt > 0);

	return num->m_type == JV_NUM;
}

int64_t jnumber_deref_i64(jvalue_ref num)
{
	int64_t result;
	ConversionResultFlags fail;
	assert(jnumber_get_i64(num, &result) == CONV_OK);
	if (CONV_OK != (fail = jnumber_get_i64(num, &result))) {
		PJ_LOG_WARN("Converting json value to a 64-bit integer but ignoring the conversion error: %d", fail);
	}
	return result;
}


raw_buffer jnumber_deref_raw(jvalue_ref num)
{
	// initialized to 0 just to get around compiler warning for
	// now - it is really up to the caller to ensure they do not
	// call this on something that is not a raw number.
	raw_buffer result = { 0 };
	assert(jnumber_get_raw(num, &result) == CONV_OK);
	jnumber_get_raw(num, &result);
	return result;
}

ConversionResultFlags jnumber_get_i32 (jvalue_ref num, int32_t *number)
{
	SANITY_CHECK_POINTER(num);

	CHECK_POINTER_RETURN_VALUE(num, CONV_BAD_ARGS);
	CHECK_POINTER_RETURN_VALUE(number, CONV_BAD_ARGS);
	CHECK_CONDITION_RETURN_VALUE(!jis_number(num), CONV_BAD_ARGS, "Trying to access %d as a number", num->m_type);

	switch (DEREF_NUM(num).m_type) {
		case NUM_FLOAT:
			return jdouble_to_i32 (DEREF_NUM(num).value.floating, number) | DEREF_NUM(num).m_error;
		case NUM_INT:
			return ji64_to_i32 (DEREF_NUM(num).value.integer, number) | DEREF_NUM(num).m_error;
		case NUM_RAW:
			assert(DEREF_NUM(num).value.raw.m_str != NULL);
			assert(DEREF_NUM(num).value.raw.m_len > 0);
			return jstr_to_i32 (&DEREF_NUM(num).value.raw, number) | DEREF_NUM(num).m_error;
		default:
			PJ_LOG_ERR("internal error - numeric type is unrecognized (%d)", (int)DEREF_NUM(num).m_type);
			assert(false);
			return CONV_GENERIC_ERROR;
	}
}

ConversionResultFlags jnumber_get_i64 (jvalue_ref num, int64_t *number)
{
	SANITY_CHECK_POINTER(num);

	CHECK_POINTER_RETURN_VALUE(num, CONV_BAD_ARGS);
	CHECK_POINTER_RETURN_VALUE(number, CONV_BAD_ARGS);
	CHECK_CONDITION_RETURN_VALUE(!jis_number(num), CONV_BAD_ARGS, "Trying to access %d as a number", num->m_type);

	switch (DEREF_NUM(num).m_type) {
		case NUM_FLOAT:
			return jdouble_to_i64 (DEREF_NUM(num).value.floating, number) | DEREF_NUM(num).m_error;
		case NUM_INT:
			*number = DEREF_NUM(num).value.integer;
			return DEREF_NUM(num).m_error;
		case NUM_RAW:
			assert(DEREF_NUM(num).value.raw.m_str != NULL);
			assert(DEREF_NUM(num).value.raw.m_len > 0);
			return jstr_to_i64 (&DEREF_NUM(num).value.raw, number) | DEREF_NUM(num).m_error;
		default:
			PJ_LOG_ERR("internal error - numeric type is unrecognized (%d)", (int)DEREF_NUM(num).m_type);
			assert(false);
			return CONV_GENERIC_ERROR;
	}
}

ConversionResultFlags jnumber_get_f64 (jvalue_ref num, double *number)
{
	SANITY_CHECK_POINTER(num);

	CHECK_POINTER_RETURN_VALUE(num, CONV_BAD_ARGS);
	CHECK_POINTER_RETURN_VALUE(number, CONV_BAD_ARGS);
	CHECK_CONDITION_RETURN_VALUE(!jis_number(num), CONV_BAD_ARGS, "Trying to access %d as a number", num->m_type);

	switch (DEREF_NUM(num).m_type) {
		case NUM_FLOAT:
			*number = DEREF_NUM(num).value.floating;
			return DEREF_NUM(num).m_error;
		case NUM_INT:
			return ji64_to_double (DEREF_NUM(num).value.integer, number) | DEREF_NUM(num).m_error;
		case NUM_RAW:
			assert(DEREF_NUM(num).value.raw.m_str != NULL);
			assert(DEREF_NUM(num).value.raw.m_len > 0);
			return jstr_to_double (&DEREF_NUM(num).value.raw, number) | DEREF_NUM(num).m_error;
		default:
			PJ_LOG_ERR("internal error - numeric type is unrecognized (%d)", (int)DEREF_NUM(num).m_type);
			assert(false);
			return CONV_GENERIC_ERROR;
	}
}

ConversionResultFlags jnumber_get_raw (jvalue_ref num, raw_buffer *result)
{
	SANITY_CHECK_POINTER(num);

	CHECK_POINTER_RETURN_VALUE(num, CONV_BAD_ARGS);
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	CHECK_CONDITION_RETURN_VALUE(!jis_number(num), CONV_BAD_ARGS, "Trying to access %d as a number", num->m_type);

	switch (DEREF_NUM(num).m_type) {
		case NUM_FLOAT:
		case NUM_INT:
			return CONV_NOT_A_RAW_NUM;
		case NUM_RAW:
			assert(DEREF_NUM(num).value.raw.m_str != NULL);
			assert(DEREF_NUM(num).value.raw.m_len > 0);
			*result = DEREF_NUM(num).value.raw;
			return CONV_OK;
		default:
			PJ_LOG_ERR("internal error - numeric type is unrecognized (%d)", (int)DEREF_NUM(num).m_type);
			assert(false);
			return CONV_GENERIC_ERROR;
	}
}

#undef DEREF_NUM
/*** JSON Boolean operations ***/
#define DEREF_BOOL(ref) ((ref)->value.val_bool)

static inline void j_destroy_boolean (jvalue_ref boolean)
{
}

static inline bool jboolean_to_string_append (jvalue_ref jref, JStreamRef generating)
{
	bool result = (generating->boolean (generating, DEREF_BOOL(jref).value) != NULL);
	if (UNLIKELY(!result)) {
		PJ_LOG_ERR("Schema validation error, bool did not validate against schema");
	}
	return result;
}

bool jis_boolean (jvalue_ref jval)
{
	SANITY_CHECK_POINTER(jval);
	assert(s_inGdb || jval->m_refCnt > 0);
	return jval->m_type == JV_BOOL;
}

jvalue_ref jboolean_create (bool value)
{
	jvalue_ref new_bool = jvalue_create (JV_BOOL);
	if (LIKELY(new_bool != NULL)) {
		DEREF_BOOL(new_bool).value = value;
	}
	return new_bool;
}

bool jboolean_deref (jvalue_ref boolean)
{
	bool result;
	assert (jis_null(boolean) || CONV_OK == jboolean_get(boolean, &result));
	jboolean_get (boolean, &result);
	return result;
}

/**
 * Retrieve the native boolean representation of this reference.
 *
 * The following equivalencies are made for the various JSON types & bool:
 * NUMBERS: 0, NaN = false, everything else = true
 * STRINGS: empty = false, non-empty = true
 * NULL: false
 * ARRAY: true
 * OBJECT: true
 * @param val The reference to the JSON value
 * @param value Where to write the boolean value to.
 * @return CONV_OK if val represents a JSON boolean type, otherwise CONV_NOT_A_BOOLEAN.
 */
ConversionResultFlags jboolean_get (jvalue_ref val, bool *value)
{
	SANITY_CHECK_POINTER(val);

	if (value) *value = false;

	CHECK_POINTER_MSG_RETURN_VALUE(val, CONV_NOT_A_BOOLEAN, "Attempting to use a C NULL as a JSON value reference");
	CHECK_POINTER_MSG_RETURN_VALUE(value, (jis_boolean(val) ? CONV_OK : CONV_NOT_A_BOOLEAN), "Non-recommended API use - value is not pointing to a valid boolean");
	assert(val->m_refCnt > 0);

	switch (val->m_type) {
		case JV_BOOL:
			if (value) *value = DEREF_BOOL(val).value;
			return CONV_OK;

		case JV_NULL:
			PJ_LOG_INFO("Attempting to convert NULL to boolean");
			if (value) *value = false;
			break;
		case JV_OBJECT:
			PJ_LOG_WARN("Attempting to convert an object to a boolean - always true");
			if (value) *value = true;
			break;
		case JV_ARRAY:
			PJ_LOG_WARN("Attempting to convert an array to a boolean - always true");
			if (value) *value = true;
			break;
		case JV_STR:
			PJ_LOG_WARN("Attempt to convert a string to a boolean - testing if string is empty");
			if (value) *value = jstring_size (val) != 0;
			break;
		case JV_NUM:
		{
			double result;
			ConversionResultFlags conv_result;
			PJ_LOG_WARN("Attempting to convert a number to a boolean - testing if number is 0");
			conv_result = jnumber_get_f64 (val, &result);
			if (value) *value = (conv_result == CONV_OK && result != 0);
			break;
		}
	}

	return CONV_NOT_A_BOOLEAN;
}

