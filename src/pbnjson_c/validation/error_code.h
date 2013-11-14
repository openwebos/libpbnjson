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

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Validation error codes */
typedef enum _ValidationErrorCode
{
	VEC_OK = 0,
	VEC_SYNTAX,
	VEC_NOT_NULL,
	VEC_NOT_ARRAY,
	VEC_ARRAY_HAS_DUPLICATES,
	VEC_ARRAY_TOO_LONG,
	VEC_ARRAY_TOO_SHORT,
	VEC_NOT_BOOLEAN,
	VEC_NOT_NUMBER,
	VEC_NOT_INTEGER_NUMBER,
	VEC_NUMBER_TOO_SMALL,
	VEC_NUMBER_TOO_BIG,
	VEC_NOT_STRING,
	VEC_STRING_TOO_SHORT,
	VEC_STRING_TOO_LONG,
	VEC_NOT_OBJECT,
	VEC_NOT_ENOUGH_KEYS,
	VEC_MISSING_REQUIRED_KEY,
	VEC_TOO_MANY_KEYS,
	VEC_OBJECT_PROPERTY_NOT_ALLOWED,
	VEC_TYPE_NOT_ALLOWED,
	VEC_MORE_THAN_ONE_OF,
	VEC_NEITHER_OF_ANY,
	VEC_NOT_EVERY_ALL_OF,
	VEC_UNEXPECTED_VALUE,
} ValidationErrorCode;

/** @brief Get human readable message for specific error code. */
char const *ValidationGetErrorMessage(int code);

#ifdef __cplusplus
}
#endif
