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

#ifndef JSCHEMA_FILE_CXX_H_
#define JSCHEMA_FILE_CXX_H_

#include "japi.h"
#include "JSchema.h"
#include "JResolver.h"

namespace pbnjson {

class JErrorHandler;

/**
 * Javascript-style comments are allowed within schemas.
 *
 * Makes an optimization in that the object will actually just
 * memory map the schema.
 *
 * Should help keep memory usage to a minimum even with lots of schemas & since
 * all this will be read-only, they are easier to swap out.
 *
 * This class is not thread safe at the moment
 * (do not share a JSchemaFile object across threads).  If you find this is a common
 * pattern, let the maintainer know.
 *
 * <palm-internal-comment>
 * Direct usage of this class will likely go away in the future - instead upper layers
 * (i.e. luna service bus) will automatically handle most schema work for you.
 *
 * For now though, this class is the recommended mechanism for utilizing schemas.
 * </palm-internal-comment>
 */
class PJSONCXX_API JSchemaFile
	: public JSchema
{
private:
	static JSchema::Resource* createSchemaMap(const std::string &path,
	                                          const std::string &rootScope,
	                                          JErrorHandler *errorHandler,
	                                          JSchemaResolverRef resolver);

public:
	/**
	 * Create a schema representation from the file with the given path.
	 *
	 * NOTE: you must supply a path representing a mmap'able file.
	 *
	 * @see isInitialized
	 */
	JSchemaFile(const std::string& path);

	/**
	 * @brief Create a schema representation from the file with external references.
	 *
	 * @param path     Path to the file
	 * @param rootScope Base URI for relative references
	 * @param errorHandler The error handlers to use when parsing the schema
	 * @param resolver The resolver to use for the schema
	 *                 (necessary if it contains external references).
	 *
	 * @see JResolver
	 */
	JSchemaFile(const std::string& path, const std::string& rootScope, JErrorHandler *errorHandler, JResolver *resolver);

	/**
	 * Copy the schema file.
	 *
	 * Behaviour is undefined if the object to copy was not initialized successfully.
	 */
	JSchemaFile(const JSchemaFile& other);

	/**
	 * Release any resources associated with this object.  This in essence
	 * is a release of the mmap if no other objects hold a map open.
	 */
	virtual ~JSchemaFile();
};

}

#endif /* JSCHEMA_FILE_CXX_H_ */
