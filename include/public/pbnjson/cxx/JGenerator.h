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

#ifndef JGENERATOR_H_
#define JGENERATOR_H_

#include "japi.h"
#include "JValue.h"
#include "JSchema.h"

namespace pbnjson {

class JResolver;

class PJSONCXX_API JGenerator {
public:
	JGenerator();
	/**
	 * @deprecated Will be removed in 3.0. Resolve schema with JSchemaFile
	 *
	 * @see JSchemaFile
	 */
	JGenerator(JResolver *resolver);
	~JGenerator();

	/**
	 * Equivalent to Javascript's JSON.stringify; converts this JSON DOM into a string ready for over-the-wire
	 * transport.
	 *
	 * @param val The JSON value to convert to a string.
	 *        NOTE: Behaviour is unspecified if this value doesn't represent an object or array. Most likely, it'll fail.
	 * @param schema The schema to use to ensure the DOM is in the correct format for over-the-wire
	 * @param asStr The stringified version of the JSON DOM
	 *
	 * @return True if the DOM was successfully stringified, false otherwise.  False typically indicates a schema violation or invalid DOM
	 *         (e.g. Root of DOM isn't an object or array)
	 */
	bool toString(const JValue &val, const JSchema &schema, std::string &asStr);

	/**
	 * Convenience function to wrap call to toString for JSON objects/arrays.
	 *
	 * Any other type will return an empty string.
	 *
	 * @param val
	 * @param schema
	 * @return The JSON string serialized to a string or the empty string on error (violated schema).
	 */
	static std::string serialize(const JValue &val, const JSchema &schema);

	/**
	 * Convenience function to wrap call to toString for JSON objects/arrays.
	 * Any other type will return an empty string.
	 *
	 * @deprecated Will be removed in 3.0. Resolve schema with JSchemaFile during schema creation.
	 *
	 * @see JSchemaFile
	 */
	static std::string serialize(const JValue &val, const JSchema &schema, JResolver *resolver);

	/**
	 * Convenience function to wrap call to toString for any JValue, without schema validation
	 *
	 * @param val
	 * @param quoteSingleString If val is type of string, returned string will be surrounded by quotes
	 * @return The JSON string serialized to a string or the empty string on error.
	 */
	static std::string serialize(const JValue &val, bool quoteSingleString);

private:
	//TODO remove in 3.0
	JResolver *m_resolver;
};

}

#endif /* JGENERATOR_H_ */
