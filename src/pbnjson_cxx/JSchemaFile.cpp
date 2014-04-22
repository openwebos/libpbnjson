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

#include <JSchemaFile.h>
#include <pbnjson.h>

#include <cstdio>
#include <cassert>

#include "../pbnjson_c/liblog.h"
#include "JSchemaResolverWrapper.h"

namespace pbnjson {

JSchema::Resource* JSchemaFile::createSchemaMap(const std::string &path, JSchemaResolverRef resolver)
{
	jschema_ref schema = jschema_parse_file_resolve(path.c_str(), NULL, resolver);
	if (schema == NULL)
		return NULL;

	return new Resource(schema, Resource::TakeSchema);
}

JSchemaFile::JSchemaFile(const std::string& path)
	: JSchema(createSchemaMap(path, NULL))
{
}

JSchemaFile::JSchemaFile(const std::string& path, JResolver *resolver)
{
	JSchemaResolverWrapper resolverWrapper(resolver);
	JSchemaResolver schemaresolver;
	schemaresolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
	schemaresolver.m_userCtxt = &resolverWrapper;

	m_resource = createSchemaMap(path, &schemaresolver);
}

JSchemaFile::JSchemaFile(const JSchemaFile& other)
	: JSchema(other)
{
}

JSchemaFile::~JSchemaFile()
{
}

}
