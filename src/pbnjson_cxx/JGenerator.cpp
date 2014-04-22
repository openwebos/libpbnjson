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

#include <JGenerator.h>
#include <JSchemaResolverWrapper.h>
#include <pbnjson.h>

#include <JResolver.h>

namespace pbnjson {

JGenerator::JGenerator()
	: m_resolver(NULL)
{
}

JGenerator::JGenerator(JResolver *resolver)
	: m_resolver(resolver)
{
}

JGenerator::~JGenerator()
{
}

bool JGenerator::toString(const JValue &obj, const JSchema& schema, std::string &asStr)
{
	if (m_resolver) {
		JSchemaResolverWrapper resolverWrapper(m_resolver);
		JSchemaResolver schemaresolver;
		schemaresolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
		schemaresolver.m_userCtxt = &resolverWrapper;
		if (!jschema_resolve_ex(schema.peek(), &schemaresolver)) {
			asStr = "";
			return false;
		}
	}
	const char *str = jvalue_tostring(obj.peekRaw(), schema.peek());

	if (str == NULL) {
		asStr = "";
		return false;
	}
	asStr = str;
	return true;
}

std::string JGenerator::serialize(const JValue &val, const JSchema &schema)
{
	JGenerator serializer;
	std::string serialized;
	serializer.toString(val, schema, serialized);
	return serialized;
}

std::string JGenerator::serialize(const JValue &val, const JSchema &schema, JResolver *resolver)
{
	JGenerator serializer(resolver);
	std::string serialized;
	if (!serializer.toString(val, schema, serialized)) {
		serialized = "";
	}
	return serialized;
}

std::string JGenerator::serialize(const JValue &val, bool quoteSingleString)
{
	const char *str = jvalue_tostring_simple(val.peekRaw());
	if (UNLIKELY(str == NULL)) {
		return "";
	}

	if (!quoteSingleString && val.isString())
	{
		size_t length = strlen(str);
		if ( (length >= 2) &&
			 (str[0] == '"') &&
			 (str[length-1] == '"') )
		{
			 return std::string(str+1, length-2);
		}
	}

	return str;
}

}
