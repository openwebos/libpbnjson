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

#include "JSchemaResolverWrapper.h"
#include "JSchema.h"
#include "JResolver.h"
#include "liblog.h"

using namespace pbnjson;

JSchemaResolverWrapper::JSchemaResolverWrapper(JResolver* resolver) : m_resolver(resolver)
{
}

JSchemaResolverWrapper::JSchemaResolverWrapper(const JSchemaResolverWrapper& other) : m_resolver(other.m_resolver)
{
}

JSchemaResolverWrapper::~JSchemaResolverWrapper()
{
}

JSchemaResolutionResult JSchemaResolverWrapper::sax_schema_resolver(JSchemaResolverRef resolver, jschema_ref *resolvedSchema)
{
	JSchemaResolverWrapper *self = static_cast<JSchemaResolverWrapper *>(resolver->m_userCtxt);
	return self->resolve(resolver, resolvedSchema);
}

JSchemaResolutionResult JSchemaResolverWrapper::resolve(JSchemaResolverRef resolver, jschema_ref *resolvedSchema)
{
	if (m_resolver == NULL) {
		PJ_LOG_ERR("Parser constructed with NULL JResolver. Unable to resolve external refs");
		return SCHEMA_GENERIC_ERROR;
	}

	if (resolver == NULL) {
		PJ_LOG_ERR("Parameter resolver is NULL. Unable to resolve external refs");
		return SCHEMA_GENERIC_ERROR;
	}

	JSchema::Resource *simpleResource = new JSchema::Resource(resolver->m_ctxt, JSchema::Resource::CopySchema);
	JSchema parent(simpleResource);

	std::string resource(resolver->m_resourceToResolve.m_str, resolver->m_resourceToResolve.m_len);

	JResolver::ResolutionRequest request(parent, resource);
	JSchemaResolutionResult result;
	JSchema resolvedWrapper(m_resolver->resolve(request, result));

	if (result == SCHEMA_RESOLVED) {
		*resolvedSchema = jschema_copy(resolvedWrapper.peek());
	} else
		*resolvedSchema = NULL;
	return result;
}

