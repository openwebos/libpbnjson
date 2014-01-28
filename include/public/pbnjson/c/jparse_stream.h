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

#ifndef JPARSE_STREAM_H_
#define JPARSE_STREAM_H_

#include <stdbool.h>
#include "japi.h"
#include "jschema.h"
#include "jobject.h"
#include "jcallbacks.h"
#include "jparse_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the DOM structure of the JSON document contained within the given file.
 *
 * @param file The c-string representing the path to parse.
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param opts The optimization mode to use when parsing the file.
 * @return An opaque reference handle to the DOM.  Use jis_valid to determine whether or
 *         not parsing succeeded.
 */
PJSON_API jvalue_ref jdom_parse_file(const char *file, JSchemaInfoRef schemaInfo, JFileOptimizationFlags opts) NON_NULL(1, 2);

/**
 * Returns the DOM structure of the JSON document.
 *
 * @param input The input string to parse.
 *              NOTE: It is unspecified if a DOM will be constructed if the input does not contain a top-level
 *              object or array.
 *              NOTE: Need not be a null-terminated string
 * @param optimizationMode Additional information about the input string that lets us optimize the creation process of the DOM.
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @return An opaque reference handle to the DOM.  Use jis_valid to determine whether or
 *         not parsing succeeded.
 */
PJSON_API jvalue_ref jdom_parse(raw_buffer input, JDOMOptimizationFlags optimizationMode, JSchemaInfoRef schemaInfo) NON_NULL(3);

/**
 * Parse the input using SAX callbacks.  Much faster in that no memory is allocated for a DOM & data is
 * processed on the fly, but less flexible & more complicated to handle in some cases.
 *
 * @param parser A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *               parsing events.
 * @param input The input string to parse
 *              NOTE: It is unspecified if the error handlers will be called if the input does not contain a top-level
 *              object or array.
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *              error handler).
 * @param data The ctxt parameter during parsing.
 *             After a successful parse, set to whatever the ctxt parameter was changed to during the parse.
 * @return True if parsing succeeded with no unrecoverable errors, false otherwise.
 *
 * @see jsax_getContext
 * @see jsax_changeContext
 */
PJSON_API bool jsax_parse_ex(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo, void **data) NON_NULL(3);

/**
 * Convenience method for the more extended version.  No error message generated.
 *
 * @see jsax_parse_ex
 */
PJSON_API bool jsax_parse(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo) NON_NULL(3);

/**
 * @see jparse_stream.c for an example of how the library uses it to implement the dom_parse functionality.
 */
PJSON_API void jsax_changeContext(JSAXContextRef saxCtxt, void *userCtxt);

/**
 * @see jparse_stream.c for an example of how the library uses it to implement the dom_parse functionality.
 */
PJSON_API void* jsax_getContext(JSAXContextRef saxCtxt);

/**
 * @brief jsaxparser_init Create and initialize SAX stream parser
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param callback A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *                 parsing events.
 * @param callback_ctxt Context that will be returned in callbacks
 * @return pointer to SAX parser
 */
jsaxparser_ref jsaxparser_create(JSchemaInfoRef schemaInfo, PJSAXCallbacks *callback, void *callback_ctxt);

/**
 * @brief jsaxparser_feed Parse part of json from input buffer
 * @param parser Pointer to SAX parser
 * @param buf Input buffer
 * @param buf_len Input buffer length
 * @return false on error
 */
PJSON_API bool jsaxparser_feed(jsaxparser_ref parser, const char *buf, int buf_len);

/**
 * @brief jsaxparser_end Finalize stream parsing
 * @param parser Pointer to SAX parser
 * @return false on error
 */
PJSON_API bool jsaxparser_end(jsaxparser_ref parser);

/**
 * @brief jsaxparser_release Release SAX parser created by jsaxparser_create
 * @param parser Pointer to SAX parser
  */
PJSON_API void jsaxparser_release(jsaxparser_ref *parser);

/**
 * @brief jsaxparser_get_error Return error description. It can be called when jsaxparser_feed/jsaxparser_end has returned false
 * @param parser Pointer to SAX parser
 * @return Pointer to string with error description. The pointer should not be released manually. It will be released in jsaxparser_release.
 */
PJSON_API const char* jsaxparser_get_error(jsaxparser_ref parser);

/**
 * @brief jdomparser_create Create and initialize DOM stream parser
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param optimizationMode Optimization flags
 * @return Pointer to DOM parser
 */
jdomparser_ref jdomparser_create(JSchemaInfoRef schemaInfo, JDOMOptimizationFlags optimizationMode);

/**
 * @brief jdomparser_feed Parse part of json from input buffer
 * @param parser Pointer to DOM parser
 * @param buf Input buffer
 * @param buf_len Input buffer length
 * @return false on error
 */
PJSON_API bool jdomparser_feed(jdomparser_ref parser, const char *buf, int buf_len);

/**
 * @brief jdomparser_end Finalyze stream parsing
 * @param parser Pointer to DOM parser
 * @return false on error
 */
PJSON_API bool jdomparser_end(jdomparser_ref parser);

/**
 * @brief jdomparser_release Release DOM parser created by jdomparser_create
 * @param parser Pointer to DOM parser
  */
PJSON_API void jdomparser_release(jdomparser_ref *parser);

/**
 * @brief jdomparser_get_error Return error description. It can be called when jdomparser_feed/jdomparser_feed has returned false
 * @param parser Pointer to DOM parser
 * @return Pointer to string with error description. The pointer should not be released manually. It will be released in jdomparser_deinit
 */
PJSON_API const char* jdomparser_get_error(jdomparser_ref parser);

/**
 * @brief jdomparser_get_result Return root jvalue for parsed json
 * @param parser Pointer to DOM parser
 * @return Root jvalue for parsed json or jinvalid if any error occured
 */
PJSON_API jvalue_ref jdomparser_get_result(jdomparser_ref parser);

#ifdef __cplusplus
}
#endif

#endif /* JPARSE_STREAM_H_ */
