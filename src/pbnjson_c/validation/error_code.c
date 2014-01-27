// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
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

char const *ValidationGetErrorMessage(int code)
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
	case VEC_ARRAY_HAS_DUPLICATES:
		return "Array has duplicates";
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
	case VEC_NUMBER_NOT_MULTIPLE_OF:
		return "Number not multiple of";
	case VEC_NOT_STRING:
		return "Not string";
	case VEC_STRING_TOO_SHORT:
		return "String too short";
	case VEC_STRING_TOO_LONG:
		return "String too long";
	case VEC_STRING_NOT_PATTERN:
		return "String doesn't match pattern";
	case VEC_NOT_OBJECT:
		return "Not object";
	case VEC_MISSING_REQUIRED_KEY:
		return "Missing required key";
	case VEC_NOT_ENOUGH_KEYS:
		return "Not enough keys";
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
	case VEC_SOME_OF_NOT:
		return "Some of not";
	case VEC_UNEXPECTED_VALUE:
		return "Unexpected value";
	default:
		return "Unknown";
	}
}

char const *SchemaGetErrorMessage(int code)
{
	switch (code)
	{
	case SEC_SYNTAX:
		return "Syntax error";
	case SEC_TYPE_FORMAT:
		return "'type' should be string or array";
	case SEC_TYPE_VALUE:
		return "'type' value should be one of the following: 'null', 'boolean', 'number', 'integer', 'string', 'array', 'object'";
	case SEC_TYPE_ARRAY_EMPTY:
		return "'type' array should contain at least one value";
	case SEC_TYPE_ARRAY_DUPLICATES:
		return "'type' array can't contain duplicate values";
	case SEC_MAXIMUM_FORMAT:
		return "'maximum' should be number";
	case SEC_MINIMUM_FORMAT:
		return "'minimum' should be number";
	case SEC_EXCLUSIVE_MAXIMUM_FORMAT:
		return "'exclusiveMaximum' should be boolean";
	case SEC_EXCLUSIVE_MINIMUM_FORMAT:
		return "'exclusiveMinimum' should be boolean";
	case SEC_MULTIPLE_OF_FORMAT:
		return "'multipleOf' should be number";
	case SEC_MULTIPLE_OF_VALUE_FORMAT:
		return "'multipleOf' value should be greater than 0";
	case SEC_MAX_LENGTH_FORMAT:
		return "'maxLength' should be number";
	case SEC_MIN_LENGTH_FORMAT:
		return "'minLength' should be number";
	case SEC_MAX_LENGTH_VALUE_FORMAT:
		return "'maxLength' value should be integer not less than 0";
	case SEC_MIN_LENGTH_VALUE_FORMAT:
		return "'minLength' value should be integer not less than 0";
	case SEC_PATTERN_FORMAT:
		return "'pattern' should be string";
	case SEC_PATTERN_VALUE_FORMAT:
		return "'pattern' value should be valid regular expression";
	case SEC_ITEMS_FORMAT:
		return "'items' should be object or array";
	case SEC_ITEMS_ARRAY_FORMAT:
		return "'items' array should contain only objects";
	case SEC_ADDITIONAL_ITEMS_FORMAT:
		return "'additionalItems' should be boolean or object";
	case SEC_MAX_ITEMS_FORMAT:
		return "'maxItems' should be number";
	case SEC_MIN_ITEMS_FORMAT:
		return "'minItems' should be number";
	case SEC_MAX_ITEMS_VALUE_FORMAT:
		return "'maxItems' value should be integer not less than 0";
	case SEC_MIN_ITEMS_VALUE_FORMAT:
		return "'minItems' value should be integer not less than 0";
	case SEC_UNIQUE_FORMAT:
		return "'unique' should be boolean";
	case SEC_PROPERTIES_FORMAT:
		return "'properties' should be object";
	case SEC_PROPERTIES_OBJECT_FORMAT:
		return "'properties' object values should be objects";
	case SEC_ADDITIONAL_PROPERTIES_FORMAT:
		return "'additionalProperties' should be boolean or object";
	case SEC_MAX_PROPERTIES_FORMAT:
		return "'maxProperties' should be number";
	case SEC_MIN_PROPERTIES_FORMAT:
		return "'minProperties' should be number";
	case SEC_MAX_PROPERTIES_VALUE_FORMAT:
		return "'maxProperties' value should be integer not less than 0";
	case SEC_MIN_PROPERTIES_VALUE_FORMAT:
		return "'minProperties' value should be integer not less than 0";
	case SEC_REQUIRED_FORMAT:
		return "'required' should be array";
	case SEC_REQUIRED_ARRAY_FORMAT:
		return "'required' array values should be strings";
	case SEC_ENUM_FORMAT:
		return "'enum' should be array";
	case SEC_ENUM_ARRAY_EMPTY:
		return "'enum' array should contain at least one value";
	case SEC_ENUM_ARRAY_DUPLICATES:
		return "'enum' array can't contain duplicate values";
	case SEC_COMBINATOR_ARRAY_FORMAT:
		return "'allOf', 'anyOf' and 'oneOf' arrays should contain only objects";
	case SEC_ALL_OF_FORMAT:
		return "'allOf' should be array";
	case SEC_ALL_OF_ARRAY_EMPTY:
		return "'allOf' array should contain at least one value";
	case SEC_ANY_OF_FORMAT:
		return "'anyOf' should be array";
	case SEC_ANY_OF_ARRAY_EMPTY:
		return "'anyOf' array should contain at least one value";
	case SEC_ONE_OF_FORMAT:
		return "'oneOf' should be array";
	case SEC_ONE_OF_ARRAY_EMPTY:
		return "'oneOf' array should contain at least one value";
	case SEC_NOT_FORMAT:
		return "'not' should be array";
	case SEC_NOT_ARRAY_EMPTY:
		return "'not' array should contain at least one value";
	case SEC_DEFINITIONS_FORMAT:
		return "'definitions' should be object";
	case SEC_DEFINITIONS_OBJECT_FORMAT:
		return "'properties' object values should be objects";
	case SEC_DSCHEMA_FORMAT:
		return "'$schema' should be string";
	case SEC_TITLE_FORMAT:
		return "'title' should be string";
	case SEC_DESCRIPTION_FORMAT:
		return "'description' should be string";
	case SEC_NAME_FORMAT:
		return "'name' should be string";
	default:
		return "Unknown";
	}
}
