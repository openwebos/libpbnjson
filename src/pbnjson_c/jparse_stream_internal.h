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

#ifndef JPARSE_STREAM_INTERNAL_H_
#define JPARSE_STREAM_INTERNAL_H_

#include <jtypes.h>
#include <jcallbacks.h>
#include <jparse_stream.h>
#include "yajl_compat.h"
#include "jschema_types_internal.h"
#include "parser_memory_pool.h"
#include "validation/validation_state.h"
#include "validation/validation_event.h"
#include "validation/validation_api.h"
#include "validation/nothing_validator.h"

int dom_null(JSAXContextRef ctxt);
int dom_boolean(JSAXContextRef ctxt, bool value);
int dom_number(JSAXContextRef ctxt, const char *number, size_t numberLen);
int dom_string(JSAXContextRef ctxt, const char *string, size_t stringLen);
int dom_object_start(JSAXContextRef ctxt);
int dom_object_key(JSAXContextRef ctxt, const char *key, size_t keyLen);
int dom_object_end(JSAXContextRef ctxt);
int dom_array_start(JSAXContextRef ctxt);
int dom_array_end(JSAXContextRef ctxt);

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

typedef struct __JSAXContext PJSAXContext;

struct jsaxparser {
	yajl_handle handle;
	PJSAXContext internalCtxt;
	yajl_callbacks yajl_cb;
	Validator *validator;
	UriResolver *uri_resolver;
	ValidationState validation_state;
	yajl_status status;
	JSchemaInfoRef schemaInfo;
	struct JErrorCallbacks errorHandler;
	char *schemaError;
	char *yajlError;
	mem_pool_t memory_pool; //should be the last field
};

struct jdomparser {
	struct jsaxparser saxparser;
	DomInfo topLevelContext;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief jsaxparser_alloc_memory Create SAX parser
 * @return pointer to SAX parser
 */
jsaxparser_ref jsaxparser_alloc_memory();

/**
 * @brief jsaxparser_init Initialize SAX stream parser
 * @param parser Parser to intialize
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param callback A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *                 parsing events.
 * @param callback_ctxt Context that will be returned in callbacks
 * @return false on error
 */
bool jsaxparser_init(jsaxparser_ref parser, JSchemaInfoRef schemaInfo, PJSAXCallbacks *callback, void *callback_ctxt);

/**
 * @brief jsaxparser_deinit Deinitialize SAX parser
 * @param parser Pointer to SAX parser
 */
void jsaxparser_deinit(jsaxparser_ref parser);

/**
 * @brief jsaxparser_free_memory Release SAX parser created by jsaxparser_alloc_memory
 * @param parser Pointer to SAX parser
 */
void jsaxparser_free_memory(jsaxparser_ref parser);

/**
 * @brief jdomparser_alloc_memory Create DOM parser
 * @return pointer to DOM parser
 */
jdomparser_ref jdomparser_alloc_memory();

/**
 * @brief jdomparser_init Initialize DOM stream parser
 * @param parser Parser to intialize
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param optimizationMode Optimization flags
 * @return false on error
 */
bool jdomparser_init(jdomparser_ref parser, JSchemaInfoRef schemaInfo, JDOMOptimizationFlags optimizationMode);

/**
 * @brief jdomparser_deinit Deinitialize DOM parser
 * @param parser Pointer to DOM parser
 */
void jdomparser_deinit(jdomparser_ref parser);

/**
 * @brief jdomparser_free_memory Release DOM parser created by jdomparser_alloc_memory
 * @param parser Pointer to DOM parser
 */
void jdomparser_free_memory(jdomparser_ref parser);

#ifdef __cplusplus
}
#endif

#endif /* JPARSE_STREAM_INTERNAL_H_ */
