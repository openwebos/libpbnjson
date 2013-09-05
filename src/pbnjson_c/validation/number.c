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

#include "number.h"
#include <stdio.h>

void number_init(Number *number)
{
	mpf_init(number->f);
}

void number_clear(Number *number)
{
	mpf_clear(number->f);
}

int number_set(Number *number, char const *str)
{
	return mpf_set_str(number->f, str, 10);
}

int number_set_n(Number *number, char const *str, size_t len)
{
	char buffer[len + 1];
	strncpy(buffer, str, len);
	buffer[len] = 0;

	return number_set(number, buffer);
}

void number_copy(Number *dest, Number *src)
{
	mpf_set(dest->f, src->f);
}

bool number_is_integer(Number const *n)
{
	return mpf_integer_p(n->f) != 0;
}

int number_compare(Number const *a, Number const *b)
{
	return mpf_cmp(a->f, b->f);
}

bool number_fits_long(Number const *n)
{
	return mpf_fits_slong_p(n->f);
}

long number_get_long(Number const *n)
{
	return mpf_get_si(n->f);
}
