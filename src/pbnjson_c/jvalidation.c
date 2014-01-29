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

#include <sys_malloc.h>
#include <sys/mman.h>
#include "jobject_internal.h"
#include "liblog.h"
#include "jvalue/num_conversion.h"
#include "jparse_stream_internal.h"
#include "validation/validation_state.h"
#include "validation/validation_event.h"
#include "validation/validation_api.h"
#include "validation/nothing_validator.h"


typedef struct {
	JErrorCallbacksRef callbacks;
	jvalue_ref jvalue;
} InnerContext;

static bool check_schema_jvalue_internal(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt);

static bool check_schema_jnull(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	ValidationEvent e = validation_event_null();
	return validation_check(&e, validation_state, ctxt);
}

//Helper function for jobject_to_string_append()
static bool check_schema_jkeyvalue(jobject_key_value jref, ValidationState *validation_state, InnerContext *ctxt)
{
	raw_buffer buf = jstring_deref(jref.key)->m_data;
	ValidationEvent e = validation_event_obj_key(buf.m_str, buf.m_len);
	if (!validation_check(&e, validation_state, ctxt))
		return false;

	return check_schema_jvalue_internal(jref.value, validation_state, ctxt);
}

static bool check_schema_jobject(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	ValidationEvent e = validation_event_obj_start();
	if (!validation_check(&e, validation_state, ctxt))
		return false;

	jobject_iter it;
	jobject_iter_init(&it, jref);
	jobject_key_value key_value;
	while (jobject_iter_next(&it, &key_value))
	{
		if (!check_schema_jkeyvalue(key_value, validation_state, ctxt))
			return false;
	}

	e = validation_event_obj_end();
	if (!validation_check(&e, validation_state, ctxt))
		return false;

	return true;
}

static bool check_schema_jarray(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	int i = 0;

	ValidationEvent e = validation_event_arr_start();
	if (!validation_check(&e, validation_state, ctxt))
		return false;

	for (i = 0; i < jarray_size(jref); i++)
	{
		jvalue_ref element = jarray_get(jref, i);
		if (!check_schema_jvalue_internal(element, validation_state, ctxt))
		{
			return false;
		}
	}

	e = validation_event_arr_end();
	// jvalue needed only for has_array_dublicates callback
	ctxt->jvalue = jref;
	if (!validation_check(&e, validation_state, ctxt))
		return false;

	return true;
}

static bool check_schema_jnumber(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	char buf[24];
	int printed;

	switch (jnum_deref(jref)->m_type)
	{
		case NUM_RAW:
			{
				raw_buffer n = jnum_deref(jref)->value.raw;
				ValidationEvent e = validation_event_number(n.m_str, n.m_len);
				return validation_check(&e, validation_state, ctxt);
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

	ValidationEvent e = validation_event_number(buf, printed);
	return validation_check(&e, validation_state, ctxt);
}

static bool check_schema_jstring(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	raw_buffer s = jstring_deref(jref)->m_data;
	ValidationEvent e = validation_event_string(s.m_str, s.m_len);
	return validation_check(&e, validation_state, ctxt);
}

static bool check_schema_jbool(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	ValidationEvent e = validation_event_boolean(jboolean_deref(jref)->value);
	return validation_check(&e, validation_state, ctxt);
}

static bool check_schema_jvalue_internal(jvalue_ref jref, ValidationState *validation_state, InnerContext *ctxt)
{
	if (UNLIKELY(!jis_valid(jref))) return false; /* errors never validates */
	switch (jref->m_type)
	{
	case JV_NULL   : return check_schema_jnull(jref, validation_state, ctxt);
	case JV_OBJECT : return check_schema_jobject(jref, validation_state, ctxt);
	case JV_ARRAY  : return check_schema_jarray(jref, validation_state, ctxt);
	case JV_NUM    : return check_schema_jnumber(jref, validation_state, ctxt);
	case JV_STR    : return check_schema_jstring(jref, validation_state, ctxt);
	case JV_BOOL   : return check_schema_jbool(jref, validation_state, ctxt);
	}

	return false;
}

void error_callback(ValidationState *s, ValidationErrorCode error, void *ctxt)
{
	JErrorCallbacksRef callbacks = ((InnerContext *) ctxt)->callbacks;
	if (!callbacks)
		return;
	if (callbacks && callbacks->m_schema)
	{
		struct __JSAXContext fake_sax_ctxt =
		{
			.m_errors = callbacks,
			.m_error_code = error,
		};
		callbacks->m_schema(callbacks->m_ctxt, &fake_sax_ctxt);
	}
}

static bool has_array_duplicates(ValidationState *s, void *ctxt)
{
	return jarray_has_duplicates(((InnerContext *) ctxt)->jvalue);
}

static Notification jvalue_check_notification =
{
	.error_func = &error_callback,
	.has_array_duplicates = &has_array_duplicates,
};

bool jvalue_check_schema(jvalue_ref jref, const JSchemaInfoRef schema_info)
{
	if (jref == NULL)
		return false;

	if ( (jref->m_type != JV_OBJECT) && (jref->m_type != JV_ARRAY) )
	{
		return false;
	}

	if (schema_info == NULL)
		return false;

	Validator *validator = NOTHING_VALIDATOR;
	UriResolver *uri_resolver = NULL;
	if (schema_info->m_schema)
	{
		validator = schema_info->m_schema->validator;
		uri_resolver = schema_info->m_schema->uri_resolver;
	}

	if (uri_resolver)
	{
		if (!jschema_resolve(schema_info))
			return false;
	}

	ValidationState validation_state = { 0 };
	validation_state_init(&validation_state,
	                      validator,
	                      uri_resolver,
	                      &jvalue_check_notification);    // TODO: report errors

	InnerContext ctxt = {
		.callbacks = schema_info->m_errHandler,
		.jvalue = jref,
	};

	bool retVal = check_schema_jvalue_internal(jref, &validation_state, &ctxt);

	validation_state_clear(&validation_state);

	return retVal;
}

