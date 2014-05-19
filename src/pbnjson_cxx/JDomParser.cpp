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

#include <JDomParser.h>
#include <pbnjson.h>
#include "JErrorHandler.h"
#include "JErrorHandlerUtils.h"
#include <JSchema.h>
#include "../pbnjson_c/jschema_types_internal.h"
#include "../pbnjson_c/validation/error_code.h"
#include "../pbnjson_c/jparse_stream_internal.h"

namespace pbnjson {

static inline raw_buffer strToRawBuffer(const std::string& str)
{
	return j_str_to_buffer(str.c_str(), str.length());
}

namespace {

bool ErrorCallbackParser(void *ctxt, JSAXContextRef parseCtxt)
{
	JDomParser *parser = static_cast<JDomParser *>(ctxt);
	JErrorHandler* handler = parser->getErrorHandler();
	if (handler)
		handler->syntax(parser, JErrorHandler::ERR_SYNTAX_GENERIC, "unknown error parsing");
	return false;
}

bool ErrorCallbackSchema(void *ctxt, JSAXContextRef parseCtxt)
{
	JDomParser *parser = static_cast<JDomParser *>(ctxt);
	JErrorHandler* handler = parser->getErrorHandler();
	if (handler)
	{
		handler->schema(parser, ErrorToSchemaError(parseCtxt->m_error_code), ValidationGetErrorMessage(parseCtxt->m_error_code));
	}
	return false;
}

bool ErrorCallbackUnkown(void *ctxt, JSAXContextRef parseCtxt)
{
	JDomParser *parser = static_cast<JDomParser *>(ctxt);
	JErrorHandler* handler = parser->getErrorHandler();
	if (handler)
		handler->misc(parser, parseCtxt->errorDescription == NULL ? "unknown error parsing" : parseCtxt->errorDescription);
	return false;
}

} //anonymous namespace

JDomParser::JDomParser()
	: JParser()
	, m_optimization(DOMOPT_NOOPT)
	, parser(NULL)
{
}

JDomParser::JDomParser(JResolver *resolver)
	: JParser(resolver)
	, m_optimization(DOMOPT_NOOPT)
	, parser(NULL)
{
}

JDomParser::~JDomParser()
{
	if (parser) {
		jdomparser_deinit(parser);
		jdomparser_free_memory(parser);
	}
}

JErrorCallbacks JDomParser::prepareCErrorCallbacks()
{
	/*
 	 *  unfortunately, I can't see a way to re-use the C++ sax parsing code
 	 *  while at the same time using the C code that builds the DOM.
 	 */
	JErrorCallbacks cErrCallbacks;
	cErrCallbacks.m_parser = ErrorCallbackParser;
	cErrCallbacks.m_schema = ErrorCallbackSchema;
	cErrCallbacks.m_unknown = ErrorCallbackUnkown;
	cErrCallbacks.m_ctxt = this;
	return cErrCallbacks;
}

bool JDomParser::parse(const std::string& input, const JSchema& schema, JErrorHandler *errors)
{
	return begin(schema, errors) && feed(input) && end();
}

bool JDomParser::begin(const JSchema &_schema, JErrorHandler *errors)
{
	if (parser)
		jdomparser_deinit(parser);
	else {
		parser = jdomparser_alloc_memory();
	}

	schema = _schema;
	externalRefResolver = prepareResolver();
	errorHandler = prepareCErrorCallbacks();
	schemaInfo = prepare(schema, externalRefResolver, errorHandler, errors);

	//TODO remove in 3.0 version
	if (oldInterface && schemaInfo.m_schema->uri_resolver && !jschema_resolve_ex(schemaInfo.m_schema, &externalRefResolver))
		return false;

	return jdomparser_init(parser, &schemaInfo, m_optimization);
}

bool JDomParser::feed(const char *buf, int length)
{
	if (!jdomparser_feed(parser, buf, length)) {
		if (getErrorHandler())
			getErrorHandler()->parseFailed(this, "parseStreamFeed failed");
		return false;
	}

	return true;
}

bool JDomParser::end()
{
	if (!jdomparser_end(parser)) {
		if (getErrorHandler())
			getErrorHandler()->parseFailed(this, "jdomparser_end failed");
		return false;
	}

	jvalue_ref jval = jdomparser_get_result(parser);
	if (!jis_valid(jval)) {
		if (getErrorHandler())
			getErrorHandler()->parseFailed(this, "parseStreamEnd failed");
		return false;
	}

	m_dom = jval;
	return true;
}

const char *JDomParser::getError()
{
	return jdomparser_get_error(parser);
}

bool JDomParser::parseFile(const std::string &file, const JSchema &schema, JFileOptimizationFlags optimization, JErrorHandler *errors)
{
	JSchemaResolver resolver = prepareResolver();
	JErrorCallbacks errCbs = prepareCErrorCallbacks();
	JSchemaInfo schemaInfo = prepare(schema, resolver, errCbs, errors);

	m_dom = jdom_parse_file(file.c_str(), &schemaInfo, optimization);

	if (m_dom.isNull()) {
		if (errors) errors->parseFailed(this, "");
		return false;
	}

	return true;
}

JValue JDomParser::getDom() {
	return m_dom;
}

}
