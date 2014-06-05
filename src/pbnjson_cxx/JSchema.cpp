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

#include <JSchema.h>
#include <JSchemaFragment.h>

#include <pbnjson.h>
#include <memory>

using namespace std;

namespace pbnjson {

#ifndef SK_DISALLOWED
#define SK_DISALLOWED "disallowed"
#endif

const JSchema& JSchema::NullSchema()
{
	static const JSchemaFragment NO_VALID_INPUT_SCHEMA(
		"{\"" SK_DISALLOWED "\":\"any\"}"
	);
	return NO_VALID_INPUT_SCHEMA;
}

const JSchema& JSchema::AllSchema()
{
	static const JSchema all_schema(jschema_all());
	return all_schema;
}

JSchema::JSchema(const JSchema& other)
	: schema(other.schema ? jschema_copy(other.schema) : NULL)
{
}

JSchema::~JSchema()
{
	jschema_release(&schema);
}

JSchema& JSchema::operator=(const JSchema& other)
{
	if (other.schema == NULL)
	{
		schema = NULL;
	}
	else if (schema != other.schema)
	{
		jschema_release(&schema);
		schema = jschema_copy(other.schema);
	}

	return *this;
}

JSchema::JSchema()
	: schema(NULL)
{
}

JSchema::JSchema(jschema_ref aSchema)
	: schema(aSchema)
{
}

bool JSchema::isInitialized() const
{
	return schema != NULL;
}

jschema_ref JSchema::peek() const
{
	return schema;
}

void JSchema::set(jschema_ref aSchema)
{
	schema = aSchema;
}

}
