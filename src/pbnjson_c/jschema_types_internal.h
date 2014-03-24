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

#ifndef JSCHEMA_TYPES_INTERNAL_H_
#define JSCHEMA_TYPES_INTERNAL_H_

#include "jparse_types.h"
#include "jgen_types.h"
#include <yajl/yajl_parse.h>


typedef struct _Validator Validator;
typedef struct _UriResolver UriResolver;
typedef struct _ValidationState ValidationState;

/**
 * This structure & any nestested structures (included jvalues)
 * cannot be changed while parsing except to resolve external references
 */
typedef struct jschema {
	int ref_count;
	Validator *validator;
	UriResolver *uri_resolver;
} jschema;


struct __JSAXContext
{
	void *ctxt;
	yajl_callbacks *m_handlers;
	JErrorCallbacksRef m_errors;
	int m_error_code;
	char *errorDescription;
	ValidationState *validation_state;
};

jschema_ref jschema_new(void);
jschema_ref jschema_copy(jschema_ref schema);
void jschema_release(jschema_ref *schema);

#endif /* JSCHEMA_TYPES_INTERNAL_H_ */
