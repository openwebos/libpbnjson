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

#pragma once

#include "validator.h"
#include "validation_event.h"
#include "validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Check a single stream token from YAJL.
 *
 * The validator doesn't detect end of stream, YAJL should.
 * Stream validation doesn't check uniqueItems or property uniqueness
 * implying this is done by the DOM.
 *
 * @param[in] e Validation event (token)
 * @param[in] s Validation state. Must be allocated before every validation.
 * @param[in] ctxt Pointer to be passed back in notifications (on error, or with default properties).
 * @return false on validation failure, true if succeeded so far.
 */
bool validation_check(ValidationEvent const *e, ValidationState *s, void *ctxt);


/** @brief Validation Error class */
typedef struct _ValidationError
{
	size_t offset;              /**< @brief Offset from the begin of the text */
	ValidationErrorCode error;  /**< @brief Error code */
} ValidationError;

/** @brief Test verbatim JSON. For unit tests. */
bool validate_json(char const *json, Validator *v,
                   UriResolver *u, ValidationError *error);

/** @brief Test verbatim JSON. For unit tests. */
bool validate_json_n(char const *json, size_t json_len, Validator *v,
                     UriResolver *u, ValidationError *error);

/** @brief Test verbatim JSON. For unit tests. */
bool validate_json_plain(char const *json, Validator *v);

#ifdef __cplusplus
}
#endif
