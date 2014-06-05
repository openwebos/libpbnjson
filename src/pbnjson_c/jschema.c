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

#include <jschema.h>
#include <jobject.h>
#include <jobject_internal.h>
#include <jparse_stream_internal.h>

#include "liblog.h"
#include "jvalue/num_conversion.h"
#include "jparse_stream_internal.h"
#include "validation/uri_resolver.h"
#include "validation/validator.h"
#include "validation/parser_api.h"
#include "validation/everything_validator.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <glib.h>
#include <stdio.h>
#include <uriparser/Uri.h>


jschema_ref jschema_new(void)
{
	jschema_ref s = g_new0(jschema, 1);
	if (!s)
		return NULL;
	s->ref_count = 1;
	s->uri_resolver = uri_resolver_new();
	if (!s->uri_resolver)
	{
		g_free(s);
		return NULL;
	}
	return s;
}

jschema_ref jschema_copy(jschema_ref schema)
{
	if (schema == jschema_all())
		return schema;
	if (schema)
		++schema->ref_count;
	return schema;
}

void jschema_release(jschema_ref *schema)
{
	jschema_ref s = *schema;
	if (!s)
		return;

	if (s == jschema_all())
	{
		SANITY_KILL_POINTER(*schema);
		return;
	}

	if (--s->ref_count)
		return;
	validator_unref(s->validator);
	uri_resolver_free(s->uri_resolver);
	g_free(s);

	SANITY_KILL_POINTER(*schema);
}

static void OnError(size_t offset, SchemaErrorCode error, char const *message, void *ctxt)
{
	JErrorCallbacksRef callbacks = (JErrorCallbacksRef) ctxt;
	if (!callbacks)
		return;
	if (callbacks && callbacks->m_parser)
	{
		struct __JSAXContext fake_sax_ctxt =
		{
			.m_errors = callbacks,
			.m_error_code = error,
			.errorDescription = (char *) message,
		};
		callbacks->m_parser(callbacks->m_ctxt, &fake_sax_ctxt);
	}
}

static bool resolve_document(jschema_ref schema,
                             char const *document,
                             JSchemaResolverRef resolver)
{
	resolver->m_ctxt = schema;
	resolver->m_resourceToResolve = j_cstr_to_buffer(document);

	jschema_ref resolved_schema = NULL;
	if (SCHEMA_RESOLVED != resolver->m_resolve(resolver, &resolved_schema))
		return false;
	if (!resolved_schema)
		return false;

	// We can lose link to the document while stealing, let's create a copy
	char doc_name[strlen(document) + 1];
	memcpy(doc_name, document, sizeof(doc_name));
	uri_resolver_steal_documents(schema->uri_resolver, resolved_schema->uri_resolver);
	// The validator may have been requested with a different document, than its path.
	uri_resolver_add_validator(schema->uri_resolver, doc_name, "#", resolved_schema->validator);
	jschema_release(&resolved_schema);
	return true;
}

static jschema_ref jschema_parse_internal(raw_buffer input,
                                          char const *root_scope,
                                          JSchemaOptimizationFlags inputOpt,
                                          JErrorCallbacksRef errorHandler,
                                          JSchemaResolverRef resolver)
{
	jschema_ref schema = jschema_new();
	if (!schema)
		return NULL;

	schema->validator = parse_schema_n(input.m_str, input.m_len,
	                                   schema->uri_resolver, root_scope,
	                                   &OnError, errorHandler);

	if (!schema->validator || (resolver && !jschema_resolve_ex(schema, resolver)))
	{
		jschema_release(&schema);
		return NULL;
	}

	return schema;
}

bool jschema_resolve(JSchemaInfoRef schema_info)
{
	return jschema_resolve_ex(schema_info->m_schema, schema_info->m_resolver);
}

bool jschema_resolve_ex(jschema_ref schema, JSchemaResolverRef resolver)
{
	assert(schema->uri_resolver);

	if (!resolver || !resolver->m_resolve)
		return false;

	char const *document_to_resolve = NULL;
	char const *prev_document_to_resolve = NULL;

	if (!resolver->m_inRecursion) {
		resolver->m_inRecursion++;
		while ((document_to_resolve = uri_resolver_get_unresolved(schema->uri_resolver)))
		{
			// It may happen, that the schema can't be parsed, so don't try to process
			// the same broken schema forever.
			if (document_to_resolve == prev_document_to_resolve)
				return false;
			prev_document_to_resolve = document_to_resolve;

			if (!resolve_document(schema, document_to_resolve, resolver))
				// We weren't able to resolve referenced document either way.
				return false;
		}
	}

	return true;
}

void jschema_info_init(JSchemaInfoRef schemaInfo, jschema_ref schema, JSchemaResolverRef resolver, JErrorCallbacksRef errHandler)
{
	// if the structure ever changes, fill the remaining with 0
	schemaInfo->m_schema = schema;
	schemaInfo->m_errHandler = errHandler;
	schemaInfo->m_resolver = resolver;
}

jschema_ref jschema_parse(raw_buffer input,
                          JSchemaOptimizationFlags inputOpt, JErrorCallbacksRef errorHandler)
{
	return jschema_parse_internal(input, "", inputOpt, errorHandler, NULL);
}

jschema_ref jschema_parse_file(const char *file, JErrorCallbacksRef errorHandler)
{
	return jschema_parse_file_resolve(file, file, errorHandler, NULL);
}

jschema_ref jschema_parse_file_resolve(const char *file, const char *rootScope, JErrorCallbacksRef errorHandler, JSchemaResolverRef resolver)
{
	// mmap the file
	const char *mapContents = NULL;
	size_t mapSize = 0;
	struct stat fileInfo;
	char fileUri[3 * strlen(file) + 8]; //Recommend size by uriparser for unix

	int fd = open(file, O_RDONLY);
	if (-1 == fd)
	{
		PJ_LOG_WARN("PBNJSON_SCHEMA_OPEN", 1, PMLOGKS("FILE", file),
		            "Unable to open schema file %s", file);
		return NULL;
	}

	if (-1 == fstat(fd, &fileInfo))
	{
		PJ_LOG_WARN("PBNJSON_SCHEMA_INFO", 1, PMLOGKS("FILE", file),
		            "Unable to get information for schema file %s", file);
		goto map_failure;
	}
	mapSize = fileInfo.st_size;

	mapContents = mmap(NULL, mapSize, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, 0);
	if (mapContents == MAP_FAILED || mapContents == NULL)
	{
		PJ_LOG_WARN("PBNJSON_SCHEMA_MMAP", 1, PMLOGKS("FILE", file),
		            "Failed to create memory map for schema file %s", file);
		mapContents = NULL;
		goto map_failure;
	}

	close(fd), fd = -1;

	if (!rootScope)
	{
		uriUnixFilenameToUriStringA(file, fileUri);
		rootScope = fileUri;
	}

	jschema_ref parsedSchema = jschema_parse_internal(j_str_to_buffer(mapContents, mapSize),
	                                                  rootScope,
	                                                  DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE,
	                                                  errorHandler,
	                                                  resolver);
	if (parsedSchema == NULL)
	{
		PJ_LOG_WARN("PBNJSON_SCHEMA_PARSE", 1, PMLOGKS("FILE", file),
		            "Failed to parse schema file %s", file);
		goto map_failure;
	}

	if (mapContents)
		munmap((char *)mapContents, mapSize), mapContents = NULL;

	if (fd != -1)
		close(fd), fd = -1;

	return parsedSchema;

map_failure:
	if (mapContents)
		munmap((char *)mapContents, mapSize);

	if (fd != -1)
		close(fd);

	return NULL;
}

jschema_ref jschema_parse_ex(raw_buffer input, JSchemaOptimizationFlags inputOpt, JSchemaInfoRef validationInfo)
{
	return jschema_parse_internal(input,
	                              "",
	                              inputOpt,
	                              validationInfo->m_errHandler,
	                              validationInfo->m_resolver);
}

static jschema JSCHEMA_ALL =
{
	.ref_count = 13,  // to let it never drop to zero
	.validator = &EVERYTHING_VALIDATOR_IMPL,
};

jschema_ref jschema_all()
{
	return &JSCHEMA_ALL;
}
