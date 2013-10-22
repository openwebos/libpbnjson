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

typedef struct _StringSpan StringSpan;
typedef struct _Validator Validator;
typedef enum _ValidatorType ValidatorType;

/** @brief Type parser errors */
enum TypeParserError
{
	TPE_OK = 0,           /**< No error */
	TPE_UNKNOWN_TYPE      /**< Unknown type keyword */
};

/** @brief Create validator that corresponds to a specific type in {"type": ...}.
 *
 * @param[in] s Type keyword in the source text
 * @param[out] error Type parser error
 * @return Validator that corresponds to the keyword
 */
Validator* type_parser_parse_simple(StringSpan const *s, enum TypeParserError *error);

/** @brief Get type numerical constant that corresponds to a specific type in {"type": ...}.
 *
 * @param[in] s Type keyword in the source text
 * @param[out] error Type parser error
 * @return Validator type enum to be used in CombinedTypesValidator
 */
ValidatorType type_parser_parse_to_type(StringSpan const *s, enum TypeParserError *error);

#ifdef __cplusplus
}
#endif
