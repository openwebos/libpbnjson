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

typedef struct _Validator Validator;

typedef struct _ParserContext
{
	Validator *validator;
	bool success;
	char const *error_message;
} ParserContext;

typedef struct _StringSpan
{
	char const *str;
	size_t str_len;
} StringSpan;

typedef union _TokenParam
{
	StringSpan string;
	bool boolean;
} TokenParam;

void parser_context_set_error(ParserContext *c, char const *error_message);

#ifdef __cplusplus
}
#endif
