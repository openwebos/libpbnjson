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
#include <string.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Arbitrary precision number */
typedef struct _Number
{
	mpf_t f;  /**< GMP is used in current implementation. */
} Number;


/** @brief Initialize a number allocated on stack. */
void number_init(Number *number);

/** @brief Deinitialize number. */
void number_clear(Number *number);

/** @brief Set number value from string.
 *
 * @param[in] number Number to operate
 * @param[in] str String to get the value from
 * @return 0 on success, -1 on failure.
 */
int number_set(Number *number, char const *str);

/** @brief Set number value from string chunk.
 *
 * @param[in] number Number to operate
 * @param[in] str Pointer to the data to get the value from
 * @param[in] len Count of bytes in the source
 * @return 0 on success, -1 on failure.
 */
int number_set_n(Number *number, char const *str, size_t len);

/** @brief Copy number to another number. */
void number_copy(Number *dest, Number *src);

/** @brief Check if number value is integer. */
bool number_is_integer(Number const *n);

/** @brief Compare two numbers.
 *
 * @return -1 if a < b, 0 if a == b, and 1 if a > b
 */
int number_compare(Number const *a, Number const *b);

/** @brief Check if a number can be fitted into long. */
bool number_fits_long(Number const *n);

/** @brief Get long from a number */
long number_get_long(Number const *n);


#ifdef __cplusplus
}
#endif
