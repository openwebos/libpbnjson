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
#include "JErrorHandlerUtils.h"
#include "../pbnjson_c/jschema_types_internal.h"
#include "../pbnjson_c/validation/error_code.h"

namespace pbnjson {

namespace {

bool OnErrorParser(void *ctxt, JSAXContextRef parseCtxt)
{
	JErrorHandler *errorHandler = static_cast<JErrorHandler *>(ctxt);
	if (errorHandler)
		errorHandler->syntax(NULL, JErrorHandler::ERR_SYNTAX_GENERIC, "error parsing");
	return false;
}

bool OnErrorSchema(void *ctxt, JSAXContextRef parseCtxt)
{
	JErrorHandler *errorHandler = static_cast<JErrorHandler *>(ctxt);
	if (errorHandler)
		errorHandler->schema(NULL, ErrorToSchemaError(parseCtxt->m_error_code), ValidationGetErrorMessage(parseCtxt->m_error_code));
	return false;
}

bool OnErrorUnknown(void *ctxt, JSAXContextRef parseCtxt)
{
	JErrorHandler *errorHandler = static_cast<JErrorHandler *>(ctxt);
	if (errorHandler)
		errorHandler->misc(NULL, "unknown error parsing");
	return false;
}

} //namespace;

JSchema::Resource*
JSchemaFile::createSchemaMap(const std::string &path, JErrorHandler *errorHandler)
{
	JErrorCallbacks error_callbacks = { 0 };
	error_callbacks.m_parser = OnErrorParser;
	error_callbacks.m_schema = OnErrorSchema;
	error_callbacks.m_unknown = OnErrorUnknown;
	error_callbacks.m_ctxt = errorHandler;

	jschema_ref schema = jschema_parse_file(path.c_str(), &error_callbacks);
	if (schema == NULL)
		return NULL;

	return new Resource(schema, Resource::TakeSchema);
}

JSchemaFile::JSchemaFile(const std::string& path)
	: JSchema(createSchemaMap(path, NULL))
{
}

JSchemaFile::JSchemaFile(const std::string &path, JErrorHandler *errorHandler)
	: JSchema(createSchemaMap(path, errorHandler))
{
}

JSchemaFile::JSchemaFile(const JSchemaFile& other)
	: JSchema(other)
{
}

JSchemaFile::~JSchemaFile()
{
}

}
