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

#ifndef JPARSER_H
#define JPARSER_H

#include "japi.h"

#include <stack>
#include <string>
#include <memory>

#include "JSchema.h"

#include "../c/jconversion.h"

namespace pbnjson {

class JErrorHandler;
class JSchema;
class JResolver;
class JSchemaResolverWrapper;

/**
 * For consistency purposes, I'm borrowing the XML terminology.  The DOM represents JSON
 * values stored as a tree in memory.  SAX represents JSON parsing that doesn't actually create any intermediary
 * representation but instead tokenizes into JSON primitives that the caller is responsible for handling.
 *
 * @see JDomParser
 */
class PJSONCXX_API JParser
{
public:
	enum NumberType {
		JNUM_CONV_RAW,
		JNUM_CONV_NATIVE,
	};

	struct ParserPosition {
		int m_line;
		int m_column;
	};

	JParser();
	/**
	 * @deprecated Will be removed in 3.0. Resolve schema with JSchemaFile
	 *
	 * @see JSchemaFile
	 */
	JParser(JResolver *schemaResolver);
	JParser(const JParser& other);
	virtual ~JParser();

	/**
	 * Parse the input using the given schema.
	 *
	 * @param input The JSON string to parse.  Must be a JSON object or an array.  Behaviour is undefined
	 *              if it isn't.  This is part of the JSON spec.
	 * @param schema The JSON schema to use when parsing.
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 * @return True if we got validly formed JSON input that was accepted by the schema, false otherwise.
	 *
	 * @see JSchema
	 * @see JSchemaFile
	 * @see JErrorHandler
	 */
	virtual bool parse(const std::string& input, const JSchema &schema, JErrorHandler *errors = NULL);

	JErrorHandler* getErrorHandler() const;
	ParserPosition getPosition() const;

	/**
	 * @brief begin Prepare class to parse json from stream
	 * @param schema Schema to validate
	 * @param errors Custom error callbacks
	 * @return false on error
	 */
	bool begin(const JSchema &schema, JErrorHandler *errors = NULL);

	/**
	 * @brief feed parse input json chunk by chunk
	 * @param buf input buffer
	 * @param length input buffer size
	 * @return false on error
	 */
	bool feed(const char *buf, int length);

	/**
	 * @brief feed parse input json chunk by chunk
	 * @param data input buffer
	 * @return false on error
	 */
	bool feed(const std::string &data) { return feed(data.data(), data.size()); }

	/**
	 * @brief end Finalize stream parsing. Final schema checks
	 * @return false on error
	 */
	bool end();

	/**
	 * @brief getError Rreturn error description if any of begin, feed or end has returned false
	 * @return error description
	 */
	char const *getError();

protected:
	/*
	 * By default, this parser will not parse any input.  You must override
	 * all the functions that might be called if you want.  Schema validation occurs
	 * just before these virtual functions are called.
	 */

	/**
	 * Called when a valid { is encountered.
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonObjectOpen() { return false; }
	/**
	 * Called when a valid property instance name is encountered
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonObjectKey(const std::string& key) { return false; }
	/**
	 * Called when a valid } is encountered
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonObjectClose() { return false; }

	/**
	 * Called when a valid [ is encountered
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonArrayOpen() { return false; }
	/**
	 * Called when a valid ] is encountered
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonArrayClose() { return false; }

	/**
	 * Called when a valid non-object-key string is encountered
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonString(const std::string& s) { return false; }

	/**
	 * Called when a valid number is encountered.  This method is invoked
	 * only if conversionToUse returns JNUM_CONV_RAW.
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonNumber(const std::string& n) { return false; }
	/**
	 * Called when a valid number is encountered.  This method is invoked
	 * only if conversionToUse returns JNUM_CONV_NATIVE and the JSON number can be
	 * converted perfectly to a 64-bit integer.  Careful because even if you expect a floating point
	 * number, you may receive an integer.
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonNumber(int64_t number) { return false; }
	/**
	 * Called when a valid number is encountered.  This method is invoked
	 * only if the number cannot be converted to a 64-bit integer.  Any errors converting this
	 * to a number are passed along (e.g. potential loss of precision, overflows, etc).
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonNumber(double &number, ConversionResultFlags asFloat) { return false; }
	/**
	 * Called when a valid boolean is encountered.
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonBoolean(bool truth) { return false; }
	/**
	 * Called when a valid null is encountered.
	 * @return Whether or not parsing should continue.
	 */
	virtual bool jsonNull() { return false; }

	/**
	 * How your parser expects numbers to be handled.
	 * JNUM_CONV_NATIVE - The numeric string is converted to a 64-bit integer or a floating point
	 *                    number for you (using proper JSON number parsing - no assumptions about
	 *                    limits like most parsers do).
	 * JNUM_CONV_RAW    - The numeric string is passed in untouched.  Use this only if you have
	 *                    some well defined need that can't be served by using native types
	 *                    (if there are any bugs in the number parser, I would recommend fixing those
	 *                    instead of trying to work around by doing it yourself).
	 *
	 * @return The number type this parser is expecting.  This controls the sax routine
	 *         that is called for number notification.
	 */
	virtual NumberType conversionToUse() const = 0;

	JErrorHandler* errorHandlers() const;
	void setErrorHandlers(JErrorHandler* errors);

protected:
	std::auto_ptr<JSchemaResolverWrapper> m_resolverWrapper;

	JSchema schema;
	JSchemaInfo schemaInfo;
	JErrorCallbacks errorHandler;
	JSchemaResolver externalRefResolver;

	JSchemaInfo prepare(const JSchema &schema, JSchemaResolver &resolver, JErrorCallbacks &cErrCbs, JErrorHandler *errors);
	JSchemaResolver prepareResolver() const;

private:
	JErrorHandler* m_errors;

	friend class SaxBounce;
	jsaxparser_ref parser;
	//TODO remove in 3.0
	bool oldInterface;
	JErrorCallbacks prepareCErrorCallbacks();
};

}

#endif /* JPARSER_H_ */
