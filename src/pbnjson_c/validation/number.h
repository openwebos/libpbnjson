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

typedef struct _Number
{
	mpf_t f;
} Number;

void number_init(Number *number);
void number_clear(Number *number);
int number_set(Number *number, char const *str);
int number_set_n(Number *number, char const *str, size_t len);
void number_copy(Number *dest, Number *src);

bool number_is_integer(Number const *n);
int number_compare(Number const *a, Number const *b);
bool number_fits_long(Number const *n);
long number_get_long(Number const *n);


int _atoi_n(const char *str, size_t len);
size_t _first_non_zero_index(const char *str, size_t len);
int _get_exp(const char *str, size_t len);
int _compare_only_nums(const char *str1, size_t len1, const char *str2, size_t len2);

#ifdef __cplusplus
}
#endif
