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

#include <JValidator.h>
#include <JSchemaResolverWrapper.h>
#include <pbnjson.h>
#include "JErrorHandlerUtils.h"
#include "../pbnjson_c/jschema_types_internal.h"
#include "../pbnjson_c/validation/error_code.h"

namespace pbnjson {

static bool err_parser(void *ctxt, JSAXContextRef parseCtxt)
{
	JErrorHandler* handler = static_cast<JErrorHandler *>(ctxt);
	if (handler)
		handler->syntax(NULL, JErrorHandler::ERR_SYNTAX_GENERIC, "unknown error parsing");
	return false;
}

static bool err_schema(void *ctxt, JSAXContextRef parseCtxt)
{
	JErrorHandler* handler = static_cast<JErrorHandler *>(ctxt);
	if (handler)
		handler->schema(NULL, ErrorToSchemaError(parseCtxt->m_error_code), ValidationGetErrorMessage(parseCtxt->m_error_code));
	return false;
}

static bool err_unknown(void *ctxt, JSAXContextRef parseCtxt)
{
	JErrorHandler* handler = static_cast<JErrorHandler *>(ctxt);
	if (handler)
		handler->misc(NULL, "unknown error parsing");
	return false;
}

bool JValidator::isValid(const JValue &jVal, const JSchema &schema, JResolver &resolver, JErrorHandler *errors) {

	JSchemaResolverWrapper resolverWrapper(&resolver);

	JSchemaResolver schemaresolver;
	schemaresolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
	schemaresolver.m_userCtxt = &resolverWrapper;
	schemaresolver.m_inRecursion = 0;

	JErrorCallbacks errorHandler;
	errorHandler.m_parser = err_parser;
	errorHandler.m_schema = err_schema;
	errorHandler.m_unknown = err_unknown;
	errorHandler.m_ctxt = errors;

	JSchemaInfo schemainfo;
	jschema_info_init(&schemainfo, schema.peek(), &schemaresolver, &errorHandler);

	return jschema_resolve_ex(schemainfo.m_schema, &schemaresolver) &&
	       jvalue_check_schema(jVal.peekRaw(), &schemainfo);
}

bool JValidator::isValid(const JValue &jVal, const JSchema &schema, JErrorHandler *errors) {

	JErrorCallbacks errorHandler;
	errorHandler.m_parser = err_parser;
	errorHandler.m_schema = err_schema;
	errorHandler.m_unknown = err_unknown;
	errorHandler.m_ctxt = errors;

	JSchemaInfo schemainfo;
	jschema_info_init(&schemainfo, schema.peek(), NULL, &errorHandler);

	return jvalue_check_schema(jVal.peekRaw(), &schemainfo);
}


bool JValidator::apply(const JValue &jVal, const JSchema &schema, JResolver *jResolver, JErrorHandler *errors)
{
	JErrorCallbacks errorHandler;
	errorHandler.m_parser = err_parser;
	errorHandler.m_schema = err_schema;
	errorHandler.m_unknown = err_unknown;
	errorHandler.m_ctxt = errors;

	if (jResolver)
	{
		JSchemaResolverWrapper resolverWrapper(jResolver);
		JSchemaResolver schemaresolver;
		schemaresolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
		schemaresolver.m_userCtxt = &resolverWrapper;
		schemaresolver.m_inRecursion = 0;

		JSchemaInfo schemainfo;
		jschema_info_init(&schemainfo, schema.peek(), &schemaresolver, &errorHandler);

		return jvalue_apply_schema(jVal.peekRaw(), &schemainfo);
	}

	JSchemaInfo schemainfo;
	jschema_info_init(&schemainfo, schema.peek(), NULL, &errorHandler);
	return jvalue_apply_schema(jVal.peekRaw(), &schemainfo);
}

}
