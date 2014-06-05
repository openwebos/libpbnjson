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

#ifndef JSCHEMA_CXX_H_
#define JSCHEMA_CXX_H_

#include "japi.h"
#include "../c/compiler/nonnull_attribute.h"
#include "../c/jschema_types.h"
#include "JValue.h"

/**
 * Not thread safe.  Do not share instances of this class
 * between threads.
 *
 * Javascript-style comments are allowed within schemas.
 *
 * This is an abstract class representing a JSON schema as defined in http://json-schema.org.
 * Schemas are used to ensure that any conversion to/from serialized form generates data
 * that is semantically valid (IPC, for large part, is the biggest visible user targetted for this feature).
 */
namespace pbnjson {

class PJSONCXX_API JSchema
{
protected:
	JSchema();
	/**
	 * Construct a schema wrapper for jschema.
	 */
	JSchema(jschema_ref schema);

	jschema_ref peek() const;
	void set(jschema_ref);

private:
	jschema_ref schema;

public:
	/**
	 * A schema that is guaranteed to never accept any input
	 * as valid.
	 */
	static const JSchema& NullSchema();

	/**
	 * A schema that is guaranteed to accept any input
	 * as valid.
	 */
	static const JSchema& AllSchema();

	/**
	 * Create a copy of the schema object.
	 */
	JSchema(const JSchema& other);
	virtual ~JSchema();

	/**
	 */
	virtual JSchema& operator=(const JSchema& other);

	virtual bool isInitialized() const;

	friend class JParser;
	friend class JGenerator;
	friend class JDomParser;
	friend class JSchemaResolverWrapper;
	friend class JValidator;
};

}

#endif /* JSCHEMA_CXX_H_ */
