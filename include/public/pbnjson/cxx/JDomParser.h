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

#ifndef JDOMPARSER_H_
#define JDOMPARSER_H_

#include "JParser.h"
#include "JValue.h"
#include "../c/jparse_types.h"

namespace pbnjson {

class JResolver;

/**
 * For consistency purposes, I'm borrowing the XML terminology.  The DOM represents JSON
 * values stored as a tree in memory.  SAX represents JSON parsing that doesn't actually create any intermediary
 * representation but instead tokenizes into JSON primitives that the caller is responsible for handling.
 *
 * @see JParser
 */
class PJSONCXX_API JDomParser : public JParser
{
public:
	/**
 	 * Initialize a JSON parser that will generate a DOM.
 	 *
 	 */
	JDomParser();

	/**
	* Initialize a JSON parser that will generate a DOM.
	*
	* @param resolver The object to use when resolving external references within a schema.
	*
	* @deprecated Will be removed in 3.0. External references resolving happens once during schema parsing with JSchemaFile
	* @see JResolver
	* @see JSchemaFile
	*/
	JDomParser(JResolver *resolver);

	virtual ~JDomParser();

	/**
	 * By default, we initialize to a conservative optimization level.
	 * Here, if you know additional information about the stream you will be parsing, you can change it.
	 *
	 * @param optLevel The optimization level to use.
	 *
	 * @see JDOMOptimization
	 */
	void changeOptimization(JDOMOptimization optLevel) { m_optimization = optLevel; }

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
	bool parse(const std::string& input, const JSchema& schema, JErrorHandler *errors = NULL);

	/**
	 * Parse the input file using the given schema.
	 * @param file The JSON string to parse.  Must be a JSON object or an array.  Behaviour is undefined
	 *             if it isn't.  This is part of the JSON spec.
	 * @param schema The JSON schema to use when parsing
	 * @param optimization The optimization level to use for parsing the file.  The JDOMOptimization is
	 *                     not used - it is implicitely set to the most optimal level since the backing buffer
	 *                     will be owned by pbnjson.
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 * @return True if we got a validly formed JSON input that was accepted by the schema, false otherwise.
	 *
	 * @see JSchema
	 * @see JSchemaFile
	 * @see JErrorHandler
	 */
	bool parseFile(const std::string& file, const JSchema& schema, ::JFileOptimizationFlags optimization = JFileOptNoOpt, JErrorHandler *errors = NULL);

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
	 * @brief getError Rreturn error description if any of begin, feed, end return false;
	 * @return error description
	 */
	const char *getError();

	/**
	 * Retrieve the "DOM" representation of the input that was last parsed by this object.
	 *
	 * @return A JValue representation of the input.
	 * @see JValue
	 */
	JValue getDom();

protected:
	JParser::NumberType conversionToUse() const { return JParser::JNUM_CONV_RAW; }

private:
	JErrorCallbacks prepareCErrorCallbacks();

	JValue m_dom;
	JDOMOptimization m_optimization;
	jdomparser_ref parser;
};

}

#endif /* JDOMPARSER_H_ */
