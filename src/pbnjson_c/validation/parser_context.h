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

/** @brief The data available in the semantic actions. */
typedef struct _ParserContext
{
	Validator *validator;       /**< @brief Root validator to be set. */
	bool success;               /**< @brief Has parsing succeeded so far? */
	char const *error_message;  /**< @brief Error message if parse failed. */
} ParserContext;

/** @brief Piece of the original text. */
typedef struct _StringSpan
{
	char const *str;  /**< @brief Pointer to the begin */
	size_t str_len;   /**< @brief Length of the span */
} StringSpan;

/** @brief Parameter for a lexical token */
typedef union _TokenParam
{
	StringSpan string;    /**< @brief Token text in the source for keys, strings, and numbers. */
	bool boolean;         /**< @brief Boolean value */
} TokenParam;

/** @brief Remember parse error */
void parser_context_set_error(ParserContext *c, char const *error_message);

#ifdef __cplusplus
}
#endif
