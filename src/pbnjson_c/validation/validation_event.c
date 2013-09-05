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

#include "validation_event.h"

ValidationEvent validation_event_null(void)
{
	ValidationEvent e =
	{
		.type = EV_NULL
	};
	return e;
}

ValidationEvent validation_event_boolean(bool val)
{
	ValidationEvent e =
	{
		.type = EV_BOOL,
		.value = {
			.boolean = val
		}
	};
	return e;
}

ValidationEvent validation_event_number(const char *str, size_t len)
{
	ValidationEvent e =
	{
		.type = EV_NUM,
		.value = {
			.string = {
				.ptr = str,
				.len = len
			}
		}
	};
	return e;
}

ValidationEvent validation_event_string(char const *str, size_t len)
{
	ValidationEvent e =
	{
		.type = EV_STR,
		.value = {
			.string = {
				.ptr = str,
				.len = len
			}
		}
	};
	return e;
}

ValidationEvent validation_event_obj_start(void)
{
	ValidationEvent e =
	{
		.type = EV_OBJ_START
	};
	return e;
}

ValidationEvent validation_event_obj_key(char const *key, size_t keylen)
{
	ValidationEvent e =
	{
		.type = EV_OBJ_KEY,
		.value = {
			.string = {
				.ptr = key,
				.len = keylen
			}
		}
	};
	return e;
}

ValidationEvent validation_event_obj_end(void)
{
	ValidationEvent e =
	{
		.type = EV_OBJ_END
	};
	return e;
}

ValidationEvent validation_event_arr_start(void)
{
	ValidationEvent e =
	{
		.type = EV_ARR_START
	};
	return e;
}

ValidationEvent validation_event_arr_end(void)
{
	ValidationEvent e =
	{
		.type = EV_ARR_END
	};
	return e;
}
