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

#include "type_parser.h"
#include "instance_types.h"
#include "parser_context.h"
#include <stdio.h>

Validator* type_parser_parse_simple(StringSpan const *s, enum TypeParserError *error)
{
	const struct InstanceTypeFactory *f = instance_type_lookup(s->str, s->str_len);
	if (!f)
	{
		if (error)
			*error = TPE_UNKNOWN_TYPE;
		return NULL;
	}

	Validator *v = f->create();
	if (!v)
	{
		if (error)
			*error = TPE_NO_MEMORY;
		return NULL;
	}
	if (error)
		*error = TPE_OK;
	return v;
}

ValidatorType type_parser_parse_to_type(StringSpan const *s, enum TypeParserError *error)
{
	const struct InstanceTypeFactory *f = instance_type_lookup(s->str, s->str_len);
	if (!f)
	{
		if (error)
			*error = TPE_UNKNOWN_TYPE;
		return V_TYPES_NUM;
	}

	return f->type;
}
