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

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Event types */
typedef enum
{
	EV_NULL = 0,    /**< JSON null */
	EV_BOOL,        /**< JSON boolean */
	EV_NUM,         /**< JSON number of any kind */
	EV_STR,         /**< JSON string */
	EV_OBJ_START,   /**< JSON start of object */
	EV_OBJ_END,     /**< JSON end of object */
	EV_OBJ_KEY,     /**< JSON object key */
	EV_ARR_START,   /**< JSON start of array */
	EV_ARR_END      /**< JSON end of array */
} ValidationEventTypes;

/** @brief Validation event data */
typedef struct _ValidationEvent
{
	ValidationEventTypes type;    /**< @brief Type of event */
	union
	{
		bool boolean;             /**< @brief Boolean parameter for JSON boolean */
		struct
		{
			char const *ptr;      /**< @brief Pointer to the start of a text */
			size_t len;           /**< @brief Length of the text */
		} string;                 /**< @brief String parameter for every JSON type except boolean */
	} value;                      /**< @brief Associated value */
} ValidationEvent;


/** @brief Create validation event for JSON null. */
ValidationEvent validation_event_null(void);

/** @brief Create validation event for JSON boolean.
 *
 * @param[in] val Boolean value
 * @return Event for validation_check()
 */
ValidationEvent validation_event_boolean(bool val);

/** @brief Create validation event for JSON number.
 *
 * @param[in] str Pointer to the number source
 * @param[in] len Length of the number
 * @return Event for validation_check()
 */
ValidationEvent validation_event_number(const char *str, size_t len);

/** @brief Create validation event for JSON string.
 *
 * @param[in] str Pointer to the string source
 * @param[in] len Length of the string
 * @return Event for validation_check()
 */
ValidationEvent validation_event_string(char const *str, size_t len);

/** @brief Create validation event for start of a JSON object.
 *
 * @return Event for validation_check()
 */
ValidationEvent validation_event_obj_start(void);

/** @brief Create validation event for a JSON object's key.
 *
 * @param[in] key Pointer to the key source
 * @param[in] keylen Length of the key
 * @return Event for validation_check()
 */
ValidationEvent validation_event_obj_key(char const *key, size_t keylen);

/** @brief Create validation event for end of a JSON object.
 *
 * @return Event for validation_check()
 */
ValidationEvent validation_event_obj_end(void);

/** @brief Create validation event for start of a JSON array.
 *
 * @return Event for validation_check()
 */
ValidationEvent validation_event_arr_start(void);

/** @brief Create validation event for end of a JSON array.
 *
 * @return Event for validation_check()
 */
ValidationEvent validation_event_arr_end(void);

#ifdef __cplusplus
}
#endif
