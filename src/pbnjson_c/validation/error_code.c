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

#include "error_code.h"

char const *ValidationGetErrorMessage(ValidationErrorCode code)
{
	switch (code)
	{
	case VEC_SYNTAX:
		return "Syntax error";
	case VEC_NOT_NULL:
		return "Not null";
	case VEC_NOT_ARRAY:
		return "Not array";
	case VEC_ARRAY_TOO_LONG:
		return "Array too long";
	case VEC_ARRAY_TOO_SHORT:
		return "Array too short";
	case VEC_NOT_BOOLEAN:
		return "Not boolean";
	case VEC_NOT_NUMBER:
		return "Not number";
	case VEC_NOT_INTEGER_NUMBER:
		return "Not integer";
	case VEC_NUMBER_TOO_SMALL:
		return "Number too small";
	case VEC_NUMBER_TOO_BIG:
		return "Number too big";
	case VEC_NOT_STRING:
		return "Not string";
	case VEC_STRING_TOO_SHORT:
		return "String too short";
	case VEC_STRING_TOO_LONG:
		return "String too long";
	case VEC_NOT_OBJECT:
		return "Not object";
	case VEC_NOT_ENOUGH_KEYS:
		return "Not enought keys";
	case VEC_TOO_MANY_KEYS:
		return "Too many keys";
	case VEC_OBJECT_PROPERTY_NOT_ALLOWED:
		return "Object property not allowed";
	case VEC_TYPE_NOT_ALLOWED:
		return "Type not allowed";
	case VEC_MORE_THAN_ONE_OF:
		return "More than one of";
	case VEC_NEITHER_OF_ANY:
		return "Neither of any";
	case VEC_NOT_EVERY_ALL_OF:
		return "Not every all of";
	case VEC_UNEXPECTED_VALUE:
		return "Unexpected value";
	default:
		return "Unknown";
	}
}
