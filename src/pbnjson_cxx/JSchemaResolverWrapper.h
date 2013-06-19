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

#include <pbnjson.h>

namespace pbnjson {

	class JResolver;
	class JSchema;

	/**
	 * A helper class to act as a middleman between pbnjson C and C++ APIs.
	 * JSchemaResolverWrapper takes a C++ JResolver object which is able to resolve external references in JSON schemas.
	 * JSchemaResolverWrapper's sax_schema_resolver function is given to C pbnjson to be used as a j_schema_resolver (resolver function).
	 * Internally sax_schema_resolver is a pass-through function for calling JResolver::resolve().
	 *
	 * @code
	 * JSchemaResolverWrapper resolverWrapper(resolver);
	 * JSchemaResolver schemaresolver;
	 * schemaresolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
	 * schemaresolver.m_userCtxt = &resolverWrapper;
	 *
	 * JSchemaInfo schemainfo;
	 * jschema_info_init(&schemainfo, schema.peek(), &schemaresolver, NULL);
	 *
	 * const char* str = jvalue_tostring_schemainfo(obj.peekRaw(), &schemainfo);
	 * @endcode
	 *
	 * schemainfo->m_resolver->m_userCtxt must be a pointer to JSchemaResolverWrapper to make sax_schema_resolver work.
	 *
	 */
	class JSchemaResolverWrapper
	{
	public:
		/**
		 * @param resolver Resolves external references in a schema. Does not take ownership of the resolver.
		 */
		JSchemaResolverWrapper(JResolver* resolver);

		JSchemaResolverWrapper(const JSchemaResolverWrapper& other);

		virtual ~JSchemaResolverWrapper();

		//This function is given as a resolver function to the C API. Wraps m_resolve->resolve().
		static JSchemaResolutionResult sax_schema_resolver(JSchemaResolverRef resolver, jschema_ref *resolvedSchema);

		/**
		 * Called when the schema should be resolved.  By default, this does not need to be overridden
		 * (it invokes the resolver you created this class with).
		 * @param abstractResolver - The resolution information from the C library
		 * @param resolvedSchema - An output variable.  It is set to the parsed external schema & ownership is
		 *                         transferred to the invoker of the callback.  This should be NULL if and only if
		 *                         the return value is not SCHEMA_RESOLVED.
		 * @return Whether or not resolution of the external schema reference succeeded.
		 */
		virtual JSchemaResolutionResult resolve(JSchemaResolverRef abstractResolver, jschema_ref *resolvedSchema);

	private:
		//Pointer to supplied resolver. sax_schema_resolver() delegates it's work here.
		JResolver* m_resolver;

	};
}
