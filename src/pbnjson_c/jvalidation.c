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
#include "jparse_stream_internal.h"


static JSAXContextRef check_schema_context_create(JSchemaInfoRef schemainfo);
static void check_schema_context_free(JSAXContextRef checkContext);
static bool check_schema_jvalue_internal (jvalue_ref jref, JSAXContextRef schemainfo);
static bool check_schema_jnull(jvalue_ref jref, JSAXContextRef checkContext);
static bool check_schema_jobject (jvalue_ref jref, JSAXContextRef checkContext);
static bool check_schema_jkeyvalue (jobject_key_value jref, JSAXContextRef checkContext);
static bool check_schema_jarray (jvalue_ref jref, JSAXContextRef generating);
static bool check_schema_jstring (jvalue_ref jref, JSAXContextRef checkContext);
static bool check_schema_jbool (jvalue_ref jref, JSAXContextRef checkContext);

static JSAXContextRef check_schema_context_create(JSchemaInfoRef schemainfo)
{
	JSAXContextRef ctxt = (JSAXContextRef)malloc(sizeof(struct __JSAXContext));
	ctxt->m_validation = jschema_init(schemainfo);
	if (ctxt->m_validation == NULL) {
		return NULL;
	}
	ctxt->m_errors = schemainfo->m_errHandler;
	ctxt->m_errorstate = jsax_error_init();
	if (schemainfo->m_errHandler != NULL)
		schemainfo->m_errHandler->m_ctxt = ctxt;

	return ctxt;
}

static void check_schema_context_free(JSAXContextRef checkContext)
{
	if(checkContext)
	{
		if(checkContext->m_validation)
			validation_destroy(&checkContext->m_validation);

		if(checkContext->m_errorstate)
			jsax_error_release(&checkContext->m_errorstate);

		if(checkContext->m_errors)
			free(checkContext->m_errors);

		free(checkContext);
	}
}

static bool check_schema_jnull(jvalue_ref jref, JSAXContextRef checkContext)
{
	return jschema_null(checkContext, checkContext->m_validation);
}

static bool check_schema_jobject (jvalue_ref jref, JSAXContextRef checkContext)
{
	if (!jschema_obj(checkContext, checkContext->m_validation)) {
		return false;
	}

	jobject_iter it;
	jobject_iter_init(&it, jref);
	jobject_key_value key_value;
	while (jobject_iter_next(&it, &key_value))
	{
		if (!check_schema_jkeyvalue(key_value, checkContext))
			return false;
	}

	if (!jschema_obj_end(checkContext, checkContext->m_validation)) {
		return false;
	}

	return true;
}

//Helper function for jobject_to_string_append()
static bool check_schema_jkeyvalue (jobject_key_value jref, JSAXContextRef checkContext)
{
	raw_buffer buf = jstring_deref(jref.key)->m_data;
	if (!jschema_key(checkContext, checkContext->m_validation, j_str_to_buffer((char *)buf.m_str, buf.m_len))) {
		return false;
	}

	return check_schema_jvalue_internal(jref.value, checkContext);
}

static bool check_schema_jarray (jvalue_ref jref, JSAXContextRef checkContext)
{
	int i = 0 ;

	if (!jschema_arr(checkContext, checkContext->m_validation)) {
		return false;
	}

	for (i = 0; i < jarray_size (jref); i++) {
		jvalue_ref element = jarray_get (jref, i);
		if (!check_schema_jvalue_internal (element, checkContext)) {
			return false;
		}
	}

	if (!jschema_arr_end(checkContext, checkContext->m_validation)) {
		return false;
	}

	return true;
}

static bool check_schema_jnumber (jvalue_ref jref, JSAXContextRef checkContext)
{
	char buf[32];
	int printed;

	switch (jnum_deref(jref)->m_type) {
		case NUM_RAW:
			return jschema_num(checkContext, checkContext->m_validation, jnum_deref(jref)->value.raw);
		case NUM_FLOAT:
			printed = snprintf(buf, sizeof(buf) - 1, "%.14lg", jnum_deref(jref)->value.floating);
			return jschema_num(checkContext, checkContext->m_validation, j_str_to_buffer(buf, printed));
			break;
		case NUM_INT:
			printed = snprintf(buf, sizeof(buf), "%" PRId64,  jnum_deref(jref)->value.integer);
			return jschema_num(checkContext, checkContext->m_validation, j_str_to_buffer(buf, printed));
		default:
			return false;
	}

	return false;
}

static bool check_schema_jstring (jvalue_ref jref, JSAXContextRef checkContext)
{
	return jschema_str(checkContext, checkContext->m_validation, jstring_deref(jref)->m_data);
}

static bool check_schema_jbool (jvalue_ref jref, JSAXContextRef checkContext)
{
	return jschema_bool(checkContext, checkContext->m_validation, jboolean_deref(jref)->value);
}

static bool check_schema_jvalue_internal (jvalue_ref jref, JSAXContextRef checkContext)
{
	switch (jref->m_type) {
		case JV_NULL   : return check_schema_jnull(jref, checkContext);
		case JV_OBJECT : return check_schema_jobject(jref, checkContext);
		case JV_ARRAY  : return check_schema_jarray (jref, checkContext);
		case JV_NUM    : return check_schema_jnumber (jref, checkContext);
		case JV_STR    : return check_schema_jstring (jref, checkContext);
		case JV_BOOL   : return check_schema_jbool (jref, checkContext);
	}

	return false;
}

bool jvalue_check_schema(jvalue_ref jref, const JSchemaInfoRef schemainfo)
{
	if (jref == NULL)
		return false;

	if ( (jref->m_type != JV_OBJECT) && (jref->m_type != JV_ARRAY) )
	{
		return false;
	}

	if (schemainfo == NULL)
		return false;

	JSAXContextRef checkContext = check_schema_context_create(schemainfo);
	if (checkContext == NULL)
		return false;

	bool retVal = check_schema_jvalue_internal(jref, checkContext);

	check_schema_context_free(checkContext);

	return retVal;
}

