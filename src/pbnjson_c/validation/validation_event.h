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

typedef enum
{
	EV_NULL = 0,
	EV_BOOL,
	EV_NUM,
	EV_STR,
	EV_OBJ_START,
	EV_OBJ_END,
	EV_OBJ_KEY,
	EV_ARR_START,
	EV_ARR_END
} ValidationEventTypes;

typedef struct _ValidationEvent
{
	ValidationEventTypes type;
	union
	{
		bool boolean;
		struct
		{
			char const *ptr;
			size_t len;
		} string;
	} value;
} ValidationEvent;

ValidationEvent validation_event_null(void);
ValidationEvent validation_event_boolean(bool val);
ValidationEvent validation_event_number(const char *str, size_t len);
ValidationEvent validation_event_string(char const *str, size_t len);
ValidationEvent validation_event_obj_start(void);
ValidationEvent validation_event_obj_key(char const *key, size_t keylen);
ValidationEvent validation_event_obj_end(void);
ValidationEvent validation_event_arr_start(void);
ValidationEvent validation_event_arr_end(void);

#ifdef __cplusplus
}
#endif
