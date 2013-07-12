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

#include <JValidator.h>
#include <JSchemaResolverWrapper.h>
#include <pbnjson.h>

namespace pbnjson {

bool JValidator::isValid(const JValue &jVal, const JSchema &schema, JResolver &resolver) {

	JSchemaResolverWrapper resolverWrapper(&resolver);

	JSchemaResolver schemaresolver;
	schemaresolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
	schemaresolver.m_userCtxt = &resolverWrapper;

	JSchemaInfo schemainfo;
	jschema_info_init(&schemainfo, schema.peek(), &schemaresolver, NULL);

	return jvalue_check_schema(jVal.peekRaw(), &schemainfo);

}

bool JValidator::isValid(const JValue &jVal, const JSchema &schema) {

	JSchemaInfo schemainfo;
	jschema_info_init(&schemainfo, schema.peek(), NULL, NULL);

	return jvalue_check_schema(jVal.peekRaw(), &schemainfo);
}

}
