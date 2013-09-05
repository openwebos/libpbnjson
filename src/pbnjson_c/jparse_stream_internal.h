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

#ifndef JPARSE_STREAM_INTERNAL_H_
#define JPARSE_STREAM_INTERNAL_H_

#include <jtypes.h>
#include <jcallbacks.h>
#include <jparse_stream.h>
#include "yajl_compat.h"
#include "jschema_types_internal.h"

PJSON_LOCAL jvalue_ref jdom_parse_ex(raw_buffer input, JDOMOptimizationFlags optimizationMode, JSchemaInfoRef schemaInfo, bool allowComments);


int dom_null(JSAXContextRef ctxt);
int dom_boolean(JSAXContextRef ctxt, bool value);
int dom_number(JSAXContextRef ctxt, const char *number, size_t numberLen);
int dom_string(JSAXContextRef ctxt, const char *string, size_t stringLen);
int dom_object_start(JSAXContextRef ctxt);
int dom_object_key(JSAXContextRef ctxt, const char *key, size_t keyLen);
int dom_object_end(JSAXContextRef ctxt);
int dom_array_start(JSAXContextRef ctxt);
int dom_array_end(JSAXContextRef ctxt);
void dom_cleanup_from_jsax(JSAXContextRef ctxt);

int my_bounce_start_map(void *ctxt);
int my_bounce_map_key(void *ctxt, const unsigned char *str, yajl_size_t strLen);
int my_bounce_end_map(void *ctxt);
int my_bounce_start_array(void *ctxt);
int my_bounce_end_array(void *ctxt);
int my_bounce_string(void *ctxt, const unsigned char *str, yajl_size_t strLen);
int my_bounce_number(void *ctxt, const char *numberVal, yajl_size_t numberLen);
int my_bounce_boolean(void *ctxt, int boolVal);
int my_bounce_null(void *ctxt);

typedef int(* pj_yajl_null )(void *ctx);
typedef int(* pj_yajl_boolean )(void *ctx, int boolVal);
typedef int(* pj_yajl_integer )(void *ctx, long integerVal);
typedef int(* pj_yajl_double )(void *ctx, double doubleVal);
typedef int(* pj_yajl_number )(void *ctx, const char *numberVal, yajl_size_t numberLen);
typedef int(* pj_yajl_string )(void *ctx, const unsigned char *stringVal, yajl_size_t stringLen);
typedef int(* pj_yajl_start_map )(void *ctx);
typedef int(* pj_yajl_map_key )(void *ctx, const unsigned char *key, yajl_size_t stringLen);
typedef int(* pj_yajl_end_map )(void *ctx);
typedef int(* pj_yajl_start_array )(void *ctx);
typedef int(* pj_yajl_end_array )(void *ctx);

typedef struct DomInfo {
	JDOMOptimization m_optInformation;
	/**
	 * This cannot be null unless we are in a top-level object or array.
	 * m_prev->m_value is the object or array that is our parent.
	 */
	struct DomInfo *m_prev;

	/**
	 * If we are setting the value for an object key, this is the key (string-type).
	 * If we are parsing the key for an object, this is NULL
	 * If we are parsing an element within an array this is NULL.
	 */
	jvalue_ref m_value;
} DomInfo;

#endif /* JPARSE_STREAM_INTERNAL_H_ */
