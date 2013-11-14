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

#include <jparse_stream.h>
#include <jobject.h>

#include <yajl/yajl_parse.h>
#include "yajl_compat.h"
#include "liblog.h"
#include "jparse_stream_internal.h"
#include "jobject_internal.h"
#include "validation/validation_state.h"
#include "validation/validation_event.h"
#include "validation/validation_api.h"
#include "validation/nothing_validator.h"
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <inttypes.h>

static bool jsax_parse_internal(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo, void **ctxt, bool logError, bool comments);

static bool file_size(int fd, off_t *s)
{
	struct stat finfo;
	if (0 != fstat(fd, &finfo)) {
		return false;
	}
	*s = finfo.st_size;
	return true;
}

static inline jvalue_ref createOptimalString(JDOMOptimization opt, const char *str, size_t strLen)
{
	if (opt == DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE)
		return jstring_create_nocopy(j_str_to_buffer(str, strLen));
	return jstring_create_copy(j_str_to_buffer(str, strLen));
}

static inline jvalue_ref createOptimalNumber(JDOMOptimization opt, const char *str, size_t strLen)
{
	if (opt == DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE)
		return jnumber_create_unsafe(j_str_to_buffer(str, strLen), NULL);
	return jnumber_create(j_str_to_buffer(str, strLen));
}

static inline DomInfo* getDOMContext(JSAXContextRef ctxt)
{
	return (DomInfo*)jsax_getContext(ctxt);
}

static inline void changeDOMContext(JSAXContextRef ctxt, DomInfo *domCtxt)
{
	jsax_changeContext(ctxt, domCtxt);
}

int dom_null(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMContext(ctxt);
	// no handle to the context
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "null encountered without any context");
	// no parent node
	CHECK_CONDITION_RETURN_VALUE(data->m_prev == NULL, 0, "unexpected state - how is this possible?");

	SANITY_CHECK_POINTER(ctxt);
	SANITY_CHECK_POINTER(data->m_prev);

	if (data->m_value == NULL) {
		CHECK_CONDITION_RETURN_VALUE(!jis_array(data->m_prev->m_value), 0, "Improper place for null");
		jarray_append(data->m_prev->m_value, jnull());
	} else if (jis_string(data->m_value)) {
		CHECK_CONDITION_RETURN_VALUE(!jis_object(data->m_prev->m_value), 0, "Improper place for null");
		jobject_put(data->m_prev->m_value, data->m_value, jnull());
		data->m_value = NULL;
	} else {
		PJ_LOG_ERR("value portion of key-value pair but not a key");
		return 0;
	}

	return 1;
}

int dom_boolean(JSAXContextRef ctxt, bool value)
{
	DomInfo *data = getDOMContext(ctxt);
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "boolean encountered without any context");
	CHECK_CONDITION_RETURN_VALUE(data->m_prev == NULL, 0, "unexpected state - how is this possible?");

	if (data->m_value == NULL) {
		CHECK_CONDITION_RETURN_VALUE(!jis_array(data->m_prev->m_value), 0, "Improper place for boolean");
		jarray_append(data->m_prev->m_value, jboolean_create(value));
	} else if (jis_string(data->m_value)) {
		CHECK_CONDITION_RETURN_VALUE(!jis_object(data->m_prev->m_value), 0, "Improper place for boolean");
		jobject_put(data->m_prev->m_value, data->m_value, jboolean_create(value));
		data->m_value = NULL;
	} else {
		PJ_LOG_ERR("value portion of key-value pair but not a key");
		return 0;
	}

	return 1;
}

int dom_number(JSAXContextRef ctxt, const char *number, size_t numberLen)
{
	DomInfo *data = getDOMContext(ctxt);
	jvalue_ref jnum;

	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "number encountered without any context");
	CHECK_CONDITION_RETURN_VALUE(data->m_prev == NULL, 0, "unexpected state - how is this possible?");
	CHECK_POINTER_RETURN_VALUE(number, 0);
	CHECK_CONDITION_RETURN_VALUE(numberLen <= 0, 0, "unexpected - numeric string doesn't actually contain a number");

	jnum = createOptimalNumber(data->m_optInformation, number, numberLen);

	if (data->m_value == NULL) {
		if (UNLIKELY(!jis_array(data->m_prev->m_value))) {
			PJ_LOG_ERR("Improper place for number");
			j_release(&jnum);
			return 0;
		}
		jarray_append(data->m_prev->m_value, jnum);
	} else if (jis_string(data->m_value)) {
		if (UNLIKELY(!jis_object(data->m_prev->m_value))) {
			PJ_LOG_ERR("Improper place for number");
			j_release(&jnum);
			return 0;
		}
		jobject_put(data->m_prev->m_value, data->m_value, jnum);
		data->m_value = NULL;
	} else {
		PJ_LOG_ERR("value portion of key-value pair but not a key");
		return 0;
	}

	return 1;
}

int dom_string(JSAXContextRef ctxt, const char *string, size_t stringLen)
{
	DomInfo *data = getDOMContext(ctxt);
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "string encountered without any context");
	CHECK_CONDITION_RETURN_VALUE(data->m_prev == NULL, 0, "unexpected state - how is this possible?");

	jvalue_ref jstr = createOptimalString(data->m_optInformation, string, stringLen);

	if (data->m_value == NULL) {
		if (UNLIKELY(!jis_array(data->m_prev->m_value))) {
			PJ_LOG_ERR("Improper place for string");
			j_release(&jstr);
			return 0;
		}
		jarray_append(data->m_prev->m_value, jstr);
	} else if (jis_string(data->m_value)) {
		if (UNLIKELY(!jis_object(data->m_prev->m_value))) {
			PJ_LOG_ERR("Improper place for string");
			j_release(&jstr);
			return 0;
		}
		jobject_put(data->m_prev->m_value, data->m_value, jstr);
		data->m_value = NULL;
	} else {
		PJ_LOG_ERR("value portion of key-value pair but not a key");
		return 0;
	}

	return 1;
}

int dom_object_start(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMContext(ctxt);
	jvalue_ref newParent;
	DomInfo *newChild;

	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "object encountered without any context");

	newParent = jobject_create();
	newChild = calloc(1, sizeof(DomInfo));

	if (UNLIKELY(newChild == NULL || jis_null(newParent))) {
		PJ_LOG_ERR("Failed to allocate space for new object");
		j_release(&newParent);
		free(newChild);
		return 0;
	}
	newChild->m_prev = data;
	newChild->m_optInformation = data->m_optInformation;
	changeDOMContext(ctxt, newChild);

	if (data->m_prev != NULL) {
		if (jis_array(data->m_prev->m_value)) {
			assert(data->m_value == NULL);
			jarray_append(data->m_prev->m_value, jvalue_copy(newParent));
		} else {
			assert(jis_object(data->m_prev->m_value));
			if (UNLIKELY(!jis_string(data->m_value)))
			{
				PJ_LOG_ERR("improper place for a child object");
				j_release(&newParent);
				return 0;
			}
			jobject_put(data->m_prev->m_value, data->m_value, jvalue_copy(newParent));
		}
	}

	data->m_value = newParent;

	return 1;
}

int dom_object_key(JSAXContextRef ctxt, const char *key, size_t keyLen)
{
	DomInfo *data = getDOMContext(ctxt);
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "object key encountered without any context");
	CHECK_CONDITION_RETURN_VALUE(data->m_value != NULL, 0, "Improper place for an object key");
	CHECK_CONDITION_RETURN_VALUE(data->m_prev == NULL, 0, "object key encountered without any parent object");
	CHECK_CONDITION_RETURN_VALUE(!jis_object(data->m_prev->m_value), 0, "object key encountered without any parent object");

	// Need to be careful here - typically, m_value isn't reference counted
	// thus if parsing fails and m_value hasn't been inserted into a bigger object that is
	// tracked, we will leak.
	// The alternate behaviour is to insert into the parent value with a null value.
	// Then when inserting the value of the key/value pair into an object, we first remove the key & re-insert
	// a key/value pair (we don't currently have a replace mechanism).
	data->m_value = createOptimalString(data->m_optInformation, key, keyLen);

	return 1;
}

int dom_object_end(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMContext(ctxt);
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "object end encountered without any context");
	CHECK_CONDITION_RETURN_VALUE(data->m_value != NULL, 0, "mismatch between key/value count");
	CHECK_CONDITION_RETURN_VALUE(!jis_object(data->m_prev->m_value), 0, "object end encountered, but not in an object");

	assert(data->m_prev != NULL);
	changeDOMContext(ctxt, data->m_prev);
	if (data->m_prev->m_prev != NULL)
	{
		j_release(&data->m_prev->m_value);
		// 0xdeadbeef may be written in debug mode, which fools the code
		data->m_prev->m_value = NULL;
	}
	free(data);

	return 1;
}

int dom_array_start(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMContext(ctxt);
	jvalue_ref newParent;
	DomInfo *newChild;
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "object encountered without any context");

	newParent = jarray_create(NULL);
	newChild = calloc(1, sizeof(DomInfo));
	if (UNLIKELY(newChild == NULL || jis_null(newParent))) {
		PJ_LOG_ERR("Failed to allocate space for new array node");
		j_release(&newParent);
		free(newChild);
		return 0;
	}
	newChild->m_prev = data;
	newChild->m_optInformation = data->m_optInformation;
	changeDOMContext(ctxt, newChild);

	if (data->m_prev != NULL) {
		if (jis_array(data->m_prev->m_value)) {
			assert(data->m_value == NULL);
			jarray_append(data->m_prev->m_value, jvalue_copy(newParent));
		} else {
			assert(jis_object(data->m_prev->m_value));
			if (UNLIKELY(!jis_string(data->m_value))) {
				PJ_LOG_ERR("improper place for a child object");
				j_release(&newParent);
				return 0;
			}
			jobject_put(data->m_prev->m_value, data->m_value, jvalue_copy(newParent));
		}
	}

	data->m_value = newParent;

	return 1;
}

int dom_array_end(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMContext(ctxt);
	CHECK_CONDITION_RETURN_VALUE(data == NULL, 0, "array end encountered without any context");
	CHECK_CONDITION_RETURN_VALUE(data->m_value != NULL, 0, "key/value for array");
	CHECK_CONDITION_RETURN_VALUE(!jis_array(data->m_prev->m_value), 0, "array end encountered, but not in an array");

	assert(data->m_prev != NULL);
	changeDOMContext(ctxt, data->m_prev);
	if (data->m_prev->m_prev != NULL)
	{
		j_release(&data->m_prev->m_value);
		data->m_prev->m_value = NULL;
	}
	free(data);

	return 1;
}

static void dom_cleanup(DomInfo *dom_info)
{
	while (dom_info)
	{
		DomInfo *cur_dom_info = dom_info;
		dom_info = dom_info->m_prev;

		j_release(&cur_dom_info->m_value);
		free(cur_dom_info);
	}
}

void dom_cleanup_from_jsax(JSAXContextRef ctxt)
{
	if (!ctxt)
		return;

	dom_cleanup(getDOMContext(ctxt));
}

jvalue_ref jdom_parse_ex(raw_buffer input, JDOMOptimizationFlags optimizationMode, JSchemaInfoRef schemaInfo, bool allowComments)
{
	jvalue_ref result;
	PJSAXCallbacks callbacks = {
		dom_object_start, // m_objStart
		dom_object_key, // m_objKey
		dom_object_end, // m_objEnd

		dom_array_start, // m_arrStart
		dom_array_end, // m_arrEnd

		dom_string, // m_string
		dom_number, // m_number
		dom_boolean, // m_boolean
		dom_null, // m_null
	};
	DomInfo *topLevelContext = calloc(1, sizeof(struct DomInfo));
	CHECK_POINTER_RETURN_NULL(topLevelContext);
	void *domCtxt = topLevelContext;
	bool parsedOK = jsax_parse_internal(&callbacks, input, schemaInfo, &domCtxt, false /* don't log errors*/, allowComments);

	result = topLevelContext->m_value;

	if (domCtxt != topLevelContext) {
		// unbalanced state machine (probably a result of parser failure)
		// cleanup so there's no memory leak
		PJ_LOG_ERR("state machine indicates invalid input");
		parsedOK = false;
		dom_cleanup(domCtxt);
		topLevelContext = NULL;
	}

	free(topLevelContext);

	if (!parsedOK) {
		PJ_LOG_ERR("Parser failure");
		return jnull();
	}

	if (result == NULL)
		PJ_LOG_ERR("result was NULL - unexpected. input was '%.*s'", (int)input.m_len, input.m_str);
	else if (result == jnull())
		PJ_LOG_WARN("result was NULL JSON - unexpected.  input was '%.*s'", (int)input.m_len, input.m_str);
	else {
		if ((optimizationMode & (DOMOPT_INPUT_NOCHANGE | DOMOPT_INPUT_OUTLIVES_DOM | DOMOPT_INPUT_NULL_TERMINATED)) && input.m_str[input.m_len] == '\0') {
			result->m_toString = (char *)input.m_str;
			result->m_toStringDealloc = NULL;
		}
	}

	return result;
}

jvalue_ref jdom_parse(raw_buffer input, JDOMOptimizationFlags optimizationMode, JSchemaInfoRef schemaInfo)
{
	return jdom_parse_ex(input, optimizationMode, schemaInfo, false);
}

jvalue_ref jdom_parse_file(const char *file, JSchemaInfoRef schemaInfo, JFileOptimizationFlags flags)
{
	CHECK_POINTER_RETURN_NULL(file);
	CHECK_POINTER_RETURN_NULL(schemaInfo);

	int fd;
	off_t fileSize;
	raw_buffer input = { 0 };
	jvalue_ref result;
	char *err_msg;

	fd = open(file, O_RDONLY);
	if (fd == -1) {
		goto errno_parse_failure;
	}

	if (!file_size(fd, &fileSize)) {
		goto errno_parse_failure;
	}

	input.m_len = fileSize;
	if (input.m_len != fileSize) {
		PJ_LOG_ERR("File too big - currently unsupported by this API");
		close(fd);
	}

	if (flags & JFileOptMMap) {
		input.m_str = (char *)mmap(NULL, input.m_len, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, 0);

		if (input.m_str == NULL || input.m_str == MAP_FAILED) {
			goto errno_parse_failure;
		}
	} else {
		input.m_str = (char *)malloc(input.m_len + 1);
		if (input.m_len != read(fd, (char *)input.m_str, input.m_len)) {
			goto errno_parse_failure;
		}
		((char *)input.m_str)[input.m_len] = 0;
	}

	result = jdom_parse(input, DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE, schemaInfo);

return_result:
	close(fd);

	if (UNLIKELY(jis_null(result))) {
		if (input.m_str) {
			if (flags & JFileOptMMap) {
				munmap((void *)input.m_str, input.m_len);
			} else {
				free((void *)input.m_str);
			}
		}
	} else {
		result->m_backingBuffer = input;
		result->m_backingBufferMMap = flags & JFileOptMMap;
	}

	return result;

errno_parse_failure:
	err_msg = strdup(strerror(errno));
	PJ_LOG_WARN("Attempt to parse json document '%s' failed (%d) : %s", file, errno, err_msg);
	free(err_msg);

	result = jnull();
	goto return_result;
}

void jsax_changeContext(JSAXContextRef saxCtxt, void *userCtxt)
{
	saxCtxt->ctxt = userCtxt;
}

void* jsax_getContext(JSAXContextRef saxCtxt)
{
	return saxCtxt->ctxt;
}

typedef struct __JSAXContext PJSAXContext;

static PJSAXCallbacks no_callbacks = { 0 };

#define DEREF_CALLBACK(callback, ...) \
	do { \
		if (callback != NULL) return callback(__VA_ARGS__); \
		return 1; \
	} while (0)


int my_bounce_start_map(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_obj_start();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_start_map, ctxt);
}

int my_bounce_map_key(void *ctxt, const unsigned char *str, yajl_size_t strLen)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_obj_key((char const *) str, strLen);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_map_key, ctxt, str, strLen);
}

int my_bounce_end_map(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_obj_end();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_end_map, ctxt);
}

int my_bounce_start_array(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_arr_start();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_start_array, ctxt);
}

int my_bounce_end_array(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_arr_end();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_end_array, ctxt);
}

int my_bounce_string(void *ctxt, const unsigned char *str, yajl_size_t strLen)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_string((char const *) str, strLen);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_string, ctxt, str, strLen);
}

int my_bounce_number(void *ctxt, const char *numberVal, yajl_size_t numberLen)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_number(numberVal, numberLen);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_number, ctxt, numberVal, numberLen);
}

int my_bounce_boolean(void *ctxt, int boolVal)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_boolean(boolVal);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_boolean, ctxt, boolVal);
}

int my_bounce_null(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;

	ValidationEvent e = validation_event_null();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	DEREF_CALLBACK(spring->m_handlers->yajl_null, ctxt);
}

static yajl_callbacks my_bounce =
{
	my_bounce_null,
	my_bounce_boolean,
	NULL, // yajl_integer,
	NULL, // yajl_double
	my_bounce_number,
	my_bounce_string,
	my_bounce_start_map,
	my_bounce_map_key,
	my_bounce_end_map,
	my_bounce_start_array,
	my_bounce_end_array,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Default property injection

static bool inject_default_jnull(jvalue_ref jref, JSAXContextRef context)
{
	if (!context->m_handlers->yajl_null)
		return true;
	return context->m_handlers->yajl_null(context);
}

static bool inject_default_jvalue(jvalue_ref jref, JSAXContextRef context);

//Helper function for jobject_to_string_append()
static bool inject_default_jkeyvalue(jobject_key_value jref, JSAXContextRef context)
{
	if (!context->m_handlers->yajl_map_key)
		return true;
	raw_buffer buf = jstring_deref(jref.key)->m_data;
	if (!context->m_handlers->yajl_map_key(context, (unsigned char const *) buf.m_str, buf.m_len))
		return false;

	return inject_default_jvalue(jref.value, context);
}

static bool inject_default_jobject(jvalue_ref jref, JSAXContextRef context)
{
	if (!context->m_handlers->yajl_start_map)
		return true;
	if (!context->m_handlers->yajl_start_map(context))
		return false;

	jobject_iter it;
	jobject_iter_init(&it, jref);
	jobject_key_value key_value;
	while (jobject_iter_next(&it, &key_value))
	{
		if (!inject_default_jkeyvalue(key_value, context))
			return false;
	}

	if (!context->m_handlers->yajl_end_map)
		return true;
	if (!context->m_handlers->yajl_end_map(context))
		return false;

	return true;
}

static bool inject_default_jarray(jvalue_ref jref, JSAXContextRef context)
{
	int i = 0;

	if (!context->m_handlers->yajl_start_array)
		return true;
	if (!context->m_handlers->yajl_start_array(context))
		return false;

	for (i = 0; i < jarray_size(jref); i++)
	{
		jvalue_ref element = jarray_get(jref, i);
		if (!inject_default_jvalue(element, context))
		{
			return false;
		}
	}

	if (!context->m_handlers->yajl_end_array)
		return true;
	if (!context->m_handlers->yajl_end_array(context))
		return false;

	return true;
}

static bool inject_default_jnumber(jvalue_ref jref, JSAXContextRef context)
{
	char buf[24];
	int printed;

	if (!context->m_handlers->yajl_number)
		return true;

	switch (jnum_deref(jref)->m_type)
	{
		case NUM_RAW:
			{
				raw_buffer n = jnum_deref(jref)->value.raw;
				if (!context->m_handlers->yajl_number(context, n.m_str, n.m_len))
					return false;
				return true;
			}
		case NUM_FLOAT:
			printed = snprintf(buf, sizeof(buf) - 1, "%.14lg", jnum_deref(jref)->value.floating);
			break;
		case NUM_INT:
			printed = snprintf(buf, sizeof(buf), "%" PRId64,  jnum_deref(jref)->value.integer);
			break;
		default:
			return false;
	}

	if (!context->m_handlers->yajl_number(context, buf, printed))
		return false;
	return true;
}

static bool inject_default_jstring(jvalue_ref jref, JSAXContextRef context)
{
	if (!context->m_handlers->yajl_string)
		return true;
	raw_buffer s = jstring_deref(jref)->m_data;
	if (!context->m_handlers->yajl_string(context, (unsigned char const *) s.m_str, s.m_len))
		return false;
	return true;
}

static bool inject_default_jbool(jvalue_ref jref, JSAXContextRef context)
{
	if (!context->m_handlers->yajl_boolean)
		return true;
	if (!context->m_handlers->yajl_boolean(context, jboolean_deref(jref)->value))
		return false;
	return true;
}

static bool inject_default_jvalue(jvalue_ref jref, JSAXContextRef context)
{
	switch (jref->m_type)
	{
	case JV_NULL   : return inject_default_jnull(jref, context);
	case JV_OBJECT : return inject_default_jobject(jref, context);
	case JV_ARRAY  : return inject_default_jarray(jref, context);
	case JV_NUM    : return inject_default_jnumber(jref, context);
	case JV_STR    : return inject_default_jstring(jref, context);
	case JV_BOOL   : return inject_default_jbool(jref, context);
	}

	return false;
}

static bool on_default_property(ValidationState *s, char const *key, jvalue_ref value, void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef) ctxt;

	if (!spring->m_handlers->yajl_map_key)
		return true;
	if (!spring->m_handlers->yajl_map_key(ctxt, (unsigned char const *) key, strlen(key)))
		return false;

	return inject_default_jvalue(value, spring);
}

static bool has_array_duplicates(ValidationState *s, void *ctxt)
{
	assert(ctxt);
	DomInfo *data = getDOMContext((JSAXContextRef) ctxt);
	assert(data && data->m_prev && data->m_prev->m_value && jis_array(data->m_prev->m_value));

	return jarray_has_duplicates(data->m_prev->m_value);
}

static void validation_error(ValidationState *s, ValidationErrorCode error, void *ctxt)
{
	assert(ctxt);
	JSAXContextRef spring = (JSAXContextRef) ctxt;
	if (spring->m_errors && spring->m_errors->m_schema)
	{
		spring->m_error_code = error;
		spring->m_errors->m_schema(spring->m_errors->m_ctxt, spring);
	}
}

static Notification jparse_notification =
{
	.default_property_func = &on_default_property,
	.has_array_duplicates = &has_array_duplicates,
	.error_func = &validation_error,
};

static bool handle_yajl_error(yajl_status parseResult,
                              yajl_handle handle,
                              raw_buffer input,
                              JSchemaInfoRef schemaInfo,
                              PJSAXContext *internalCtxt)
{
	switch (parseResult)
	{
	case yajl_status_ok:
		return true;
	case yajl_status_client_canceled:
		if (!schemaInfo || !schemaInfo->m_errHandler ||
		    !schemaInfo->m_errHandler->m_unknown(schemaInfo->m_errHandler->m_ctxt, internalCtxt))
		{
			return false;
		}
		PJ_LOG_WARN("Client claims they handled an unknown error in '%.*s'", (int)input.m_len, input.m_str);
		return true;
#if YAJL_VERSION < 20000
	case yajl_status_insufficient_data:
		if (!schemaInfo || !schemaInfo->m_errHandler ||
		    !schemaInfo->m_errHandler->m_parser(schemaInfo->m_errHandler->m_ctxt, internalCtxt))
		{
			return false;
		}
		PJ_LOG_WARN("Client claims they handled incomplete JSON input provided '%.*s'", (int)input.m_len, input.m_str);
		return true;
#endif
	case yajl_status_error:
	default:
		internalCtxt->errorDescription = (char*)yajl_get_error(handle, 1, (unsigned char *)input.m_str, input.m_len);;
		if (!schemaInfo || !schemaInfo->m_errHandler ||
		    !schemaInfo->m_errHandler->m_unknown(schemaInfo->m_errHandler->m_ctxt, internalCtxt))
		{
			yajl_free_error(handle, (unsigned char*)internalCtxt->errorDescription);
			return false;
		}
		yajl_free_error(handle, (unsigned char*)internalCtxt->errorDescription);

		PJ_LOG_WARN("Client claims they handled an unknown error in '%.*s'", (int)input.m_len, input.m_str);
		return true;
	}
}

static bool jsax_parse_internal(PJSAXCallbacks *parser,
                                raw_buffer input,
                                JSchemaInfoRef schemaInfo,
                                void **ctxt,
                                bool logError,
                                bool comments)
{
	yajl_status parseResult;

	PJ_LOG_TRACE("Parsing '%.*s'", RB_PRINTF(input));

	if (parser == NULL)
		parser = &no_callbacks;

	yajl_callbacks yajl_cb =
	{
		(pj_yajl_null)parser->m_null, // yajl_null
		(pj_yajl_boolean)parser->m_boolean, // yajl_boolean
		NULL, // yajl_integer
		NULL, // yajl_double
		(pj_yajl_number)parser->m_number, // yajl_number
		(pj_yajl_string)parser->m_string, // yajl_stirng
		(pj_yajl_start_map)parser->m_objStart, // yajl_start_map
		(pj_yajl_map_key)parser->m_objKey, // yajl_map_key
		(pj_yajl_end_map)parser->m_objEnd, // yajl_end_map
		(pj_yajl_start_array)parser->m_arrStart, // yajl_start_array
		(pj_yajl_end_array)parser->m_arrEnd, // yajl_end_array
	};

	Validator *validator = NOTHING_VALIDATOR;
	UriResolver *uri_resolver = NULL;
	if (schemaInfo->m_schema)
	{
		validator = schemaInfo->m_schema->validator;
		uri_resolver = schemaInfo->m_schema->uri_resolver;
	}

	if (uri_resolver)
	{
		if (!jschema_resolve(schemaInfo))
			return false;
	}

	ValidationState validation_state = { 0 };
	validation_state_init(&validation_state,
	                      validator,
	                      uri_resolver,
	                      &jparse_notification); // TODO: Error callback

	PJSAXContext internalCtxt =
	{
		.ctxt = (ctxt != NULL ? *ctxt : NULL),
		.m_handlers = &yajl_cb,
		.m_errors = schemaInfo->m_errHandler,
		.m_error_code = 0,
		.errorDescription = NULL,
		.validation_state = &validation_state,
	};

#if YAJL_VERSION < 20000
	yajl_parser_config yajl_opts =
	{
		comments,
		0, // currently only UTF-8 will be supported for input.
	};

	yajl_handle handle = yajl_alloc(&my_bounce, &yajl_opts, NULL, &internalCtxt);
#else
	yajl_handle handle = yajl_alloc(&my_bounce, NULL, &internalCtxt);

	yajl_config(handle, yajl_allow_comments, comments ? 1 : 0);

	// currently only UTF-8 will be supported for input.
	yajl_config(handle, yajl_dont_validate_strings, 0);
#endif // YAJL_VERSION
	parseResult = yajl_parse(handle, (unsigned char *)input.m_str, input.m_len);
	if (ctxt != NULL)
	{
		*ctxt = jsax_getContext(&internalCtxt);
	}

	if (
#if YAJL_VERSION < 20000
	    yajl_status_insufficient_data != parseResult &&
#endif // YAJL_VERSION
	    !handle_yajl_error(parseResult,
	                       handle,
	                       input,
	                       schemaInfo,
	                       &internalCtxt))
		goto parse_failure;

#if YAJL_VERSION < 20000
	parseResult = yajl_parse_complete(handle);
#else
	parseResult = yajl_complete_parse(handle);
#endif // YAJL_VERSION
	if (!handle_yajl_error(parseResult,
	                       handle,
	                       input,
	                       schemaInfo,
	                       &internalCtxt))
		goto parse_failure;


	validation_state_clear(&validation_state);
	yajl_free(handle);
	return true;

parse_failure:
	if (UNLIKELY(logError))
	{
		unsigned char *errMsg = yajl_get_error(handle, 1, (unsigned char *)input.m_str, input.m_len);
		PJ_LOG_WARN("Parser reason for failure:\n'%s'", errMsg);
		yajl_free_error(handle, errMsg);
	}
	validation_state_clear(&validation_state);
	yajl_free(handle);
	return false;
}

bool jsax_parse_ex(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo, void **ctxt, bool logError)
{
	return jsax_parse_internal(parser, input, schemaInfo, ctxt, logError, false);
}

bool jsax_parse(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schema)
{
	assert(schema != NULL);
	return jsax_parse_ex(parser, input, schema, NULL, false);
}
