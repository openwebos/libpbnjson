/* @@@LICENSE
*
*      Copyright (c) 2012-2014 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
LICENSE@@@ */

#include <JParser.h>
#include <JErrorHandler.h>
#include <pbnjson.h>
#include <JResolver.h>
#include <JSchemaResolverWrapper.h>
#include "liblog.h"
#include "../pbnjson_c/jparse_stream_internal.h"

namespace pbnjson {

/**
 * Need this class to get around the member visibility restriction (& not having to declare all of the functions as
 * friends).
 */
class PJSONCXX_LOCAL SaxBounce {
public:
	static inline bool oo(JParser *p) { return p->jsonObjectOpen(); }
	static inline bool ok(JParser *p, const std::string& key) { return p->jsonObjectKey(key); }
	static inline bool oc(JParser *p) { return p->jsonObjectClose(); }
	static inline bool ao(JParser *p) { return p->jsonArrayOpen(); }
	static inline bool ac(JParser *p) { return p->jsonArrayClose(); }

	static inline bool s(JParser *p, const std::string& s) { return p->jsonString(s); }
	static inline bool n(JParser *p, const std::string& n) { return p->jsonNumber(n); }
	static inline bool n(JParser *p, int64_t n) { return p->jsonNumber(n); }
	static inline bool n(JParser *p, double n, ConversionResultFlags f) { return p->jsonNumber(n, f); }
	static inline bool b(JParser *p, bool v) { return p->jsonBoolean(v); }
	static inline bool N(JParser *p) { return p->jsonNull(); }
	static inline JParser::NumberType conversionToUse(JParser * const p) { return p->conversionToUse(); }

};

static int __obj_start(JSAXContextRef ctxt)
{
	return SaxBounce::oo(static_cast<JParser *>(jsax_getContext(ctxt)));
}

static int __obj_key(JSAXContextRef ctxt, const char *key, size_t len)
{
	return SaxBounce::ok(static_cast<JParser *>(jsax_getContext(ctxt)), std::string(key, len));
}

static int __obj_end(JSAXContextRef ctxt)
{
	return SaxBounce::oc(static_cast<JParser *>(jsax_getContext(ctxt)));
}

static int __arr_start(JSAXContextRef ctxt)
{
	return SaxBounce::ao(static_cast<JParser *>(jsax_getContext(ctxt)));
}

static int __arr_end(JSAXContextRef ctxt)
{
	return SaxBounce::ac(static_cast<JParser *>(jsax_getContext(ctxt)));
}

static int __string(JSAXContextRef ctxt, const char *str, size_t len)
{
	return SaxBounce::s(static_cast<JParser *>(jsax_getContext(ctxt)), std::string(str, len));
}

static int __number(JSAXContextRef ctxt, const char *number, size_t len)
{
	JParser *p = static_cast<JParser *>(jsax_getContext(ctxt));
	switch (SaxBounce::conversionToUse(p)) {
		case JParser::JNUM_CONV_RAW:
			return SaxBounce::n(p, std::string(number, len));
		case JParser::JNUM_CONV_NATIVE:
		{
			jvalue_ref toConv = jnumber_create_unsafe(j_str_to_buffer(number, len), NULL);
			int64_t asInteger;
			double asFloat;
			ConversionResultFlags toFloatErrors;

			if (CONV_OK == jnumber_get_i64(toConv, &asInteger)) {
				j_release(&toConv);
				return SaxBounce::n(p, (asInteger));
			}
			toFloatErrors = jnumber_get_f64(toConv, &asFloat);
			j_release(&toConv);
			return SaxBounce::n(p, asFloat, toFloatErrors);
		}
		default:
			PJ_LOG_ERR("Actual parser hasn't told us a valid type for how it wants numbers presented to it");
			return 0;
	}
}

static int __boolean(JSAXContextRef ctxt, bool value)
{
	return SaxBounce::b(static_cast<JParser *>(jsax_getContext(ctxt)), value);
}

static int __jnull(JSAXContextRef ctxt)
{
	return SaxBounce::N(static_cast<JParser *>(jsax_getContext(ctxt)));
}

static PJSAXCallbacks callbacks = {
	__obj_start, __obj_key, __obj_end, __arr_start, __arr_end, __string, __number, __boolean, __jnull,
};

namespace {

bool ErrorCallbackParser(void *ctxt, JSAXContextRef parseCtxt)
{
	JParser *parser = static_cast<JParser *>(jsax_getContext(parseCtxt));
	JErrorHandler* handler = static_cast<JErrorHandler *>(ctxt);
	if (handler)
		handler->syntax(parser, JErrorHandler::ERR_SYNTAX_GENERIC, "unknown error parsing");
	return false;
}

bool ErrorCallbackSchema(void *ctxt, JSAXContextRef parseCtxt)
{
	JParser *parser = static_cast<JParser *>(jsax_getContext(parseCtxt));
	JErrorHandler* handler = static_cast<JErrorHandler *>(ctxt);
	if (handler)
		handler->schema(parser, JErrorHandler::ERR_SCHEMA_GENERIC, "unknown schema violation parsing");
	return false;
}

bool ErrorCallbackUnknown(void *ctxt, JSAXContextRef parseCtxt)
{
	JParser *parser = static_cast<JParser *>(jsax_getContext(parseCtxt));
	JErrorHandler* handler = static_cast<JErrorHandler *>(ctxt);
	if (handler)
		handler->misc(parser, "unknown error parsing");
	return false;
}

}

static inline raw_buffer strToRawBuffer(const std::string& str)
{
	return j_str_to_buffer(str.c_str(), str.length());
}

JParser::JParser(JResolver* schemaResolver)
	: m_resolverWrapper(new JSchemaResolverWrapper(schemaResolver))
	, schema(JSchema::AllSchema())
	, m_errors(NULL)
	, parser(NULL)
{
}

JParser::JParser(const JParser& other)
	: m_resolverWrapper(new JSchemaResolverWrapper(*other.m_resolverWrapper))
	, schema(JSchema::AllSchema())
	, m_errors(other.m_errors)
	, parser(NULL)
{
}

JParser::~JParser()
{
	if (parser) {
		jsaxparser_deinit(parser);
		jsaxparser_free_memory(parser);
	}
}

JSchemaInfo JParser::prepare(const JSchema& schema, JSchemaResolver& resolver, JErrorCallbacks& cErrCbs, JErrorHandler *errors)
{
	JSchemaInfo schemaInfo;

	jschema_info_init(&schemaInfo,
	                  schema.peek(),
	                  &resolver,
	                  &cErrCbs);

	setErrorHandlers(errors);

	return schemaInfo;
}


JSchemaResolver JParser::prepareResolver() const
{
	JSchemaResolver resolver;
	resolver.m_resolve = &(m_resolverWrapper->sax_schema_resolver);
	resolver.m_userCtxt = m_resolverWrapper.get();
	return resolver;
}

JErrorCallbacks JParser::prepareCErrorCallbacks()
{
	/*
	*  unfortunately, I can't see a way to re-use the C++ sax parsing code
	*  while at the same time using the C code that builds the DOM.
	*/
	JErrorCallbacks cErrCallbacks;
	cErrCallbacks.m_parser = ErrorCallbackParser;
	cErrCallbacks.m_schema = ErrorCallbackSchema;
	cErrCallbacks.m_unknown = ErrorCallbackUnknown;
	cErrCallbacks.m_ctxt = this;
	return cErrCallbacks;
}

bool JParser::begin(const JSchema &_schema, JErrorHandler *errors)
{
	if (parser)
		jsaxparser_deinit(parser);
	else {
		parser = jsaxparser_alloc_memory();
	}
		
	schema = _schema;
	externalRefResolver = prepareResolver();
	errorHandler = prepareCErrorCallbacks();
	schemaInfo = prepare(schema, externalRefResolver, errorHandler, errors);
	jschema_info_init(&schemaInfo, schema.peek(), &externalRefResolver, &errorHandler);

	return jsaxparser_init(parser, &schemaInfo, &callbacks, this);
}

bool JParser::feed(const char *buf, int length)
{
	return jsaxparser_feed(parser, buf, length);
}

bool JParser::end()
{
	return jsaxparser_end(parser);
}

char const *JParser::getError()
{
	return jsaxparser_get_error(parser);
}

bool JParser::parse(const std::string& input, const JSchema& schema, JErrorHandler *errors)
{
	jsaxparser_ref prev = parser;

	jsaxparser parser_memory;
	memset(&parser_memory, 0, sizeof(jsaxparser));
	parser = &parser_memory;

	if (begin(schema, errors) && feed(input) && end()) {
		jsaxparser_deinit(parser);
		parser = prev;
		return true;
	}

	jsaxparser_deinit(parser);
	parser = prev;

	return false;
}

JErrorHandler* JParser::errorHandlers() const
{
	return m_errors;
}

void JParser::setErrorHandlers(JErrorHandler* errors)
{
	m_errors = errors;
}

JErrorHandler* JParser::getErrorHandler() const
{
	return m_errors;
}

JParser::ParserPosition JParser::getPosition() const
{
	return (JParser::ParserPosition){ -1, -1 };
}

}
