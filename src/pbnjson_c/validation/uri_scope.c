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

#include "uri_scope.h"
#include <string.h>
#include <assert.h>
#include <uriparser/Uri.h>

char const *const ROOT_FRAGMENT = "#";
char const *const ROOT_DEFINITIONS = "#/definitions";

UriScope *uri_scope_new(void)
{
	UriScope *u = g_new0(UriScope, 1);
	return u;
}

static void _free_uri(gpointer d)
{
	uriFreeUriMembersA((UriUriA *) d);
	g_free(d);
}

void uri_scope_free(UriScope *u)
{
	if (!u)
		return;
	g_slist_free_full(u->uri_stack, _free_uri);
	g_free(u);
}

static UriUriA *uri_scope_get_uri_unsafe(UriScope const *u)
{
	assert(u->uri_stack);
	return (UriUriA *) u->uri_stack->data;
}

static UriUriA *uri_scope_get_uri(UriScope const *u)
{
	if (!u->uri_stack)
		return NULL;
	return uri_scope_get_uri_unsafe(u);
}

char const *uri_scope_get_fragment(UriScope const *u)
{
	UriUriA *uri = uri_scope_get_uri_unsafe(u);
	return uri->fragment.first ? uri->fragment.first - 1 : ROOT_FRAGMENT;
}

int uri_scope_get_document_length(UriScope const *u)
{
	UriUriA *uri = uri_scope_get_uri_unsafe(u);
	int result = 0;

	UriTextRangeA fragment = uri->fragment;
	uri->fragment = (UriTextRangeA){};

	if (URI_SUCCESS != uriToStringCharsRequiredA(uri, &result))
	{
		uri->fragment = fragment;
		return -1;
	}
	uri->fragment = fragment;
	return result + 1;
}

char const *uri_scope_get_document(UriScope const *u, char *buffer, int chars_required)
{
	UriUriA *uri = uri_scope_get_uri_unsafe(u);

	// we need to re-construct uri without fragment
	// to do that we temporary nullify its pointers
	UriTextRangeA fragment = uri->fragment;
	uri->fragment = (UriTextRangeA){};

	if (URI_SUCCESS != uriToStringA(buffer, uri, chars_required, NULL))
	{
		uri->fragment = fragment;
		return NULL;
	}
	uri->fragment = fragment; // restore fragment
	return buffer;
}

char const *escape_json_pointer(char const *fragment, size_t fragment_len, char *buffer)
{
	assert(fragment);

	char *cur_pos = buffer;
	const char *in_pos = fragment;
	const char *in_end = in_pos + fragment_len;
	for (; in_pos != in_end; ++in_pos)
	{
		char ch = *in_pos;
		assert(ch != '\0');
		switch (ch)
		{
		case '~':
			*cur_pos++ = '~';
			*cur_pos++ = '0';
			break;
		case '/':
			*cur_pos++ = '~';
			*cur_pos++ = '1';
			break;
		default:
			*cur_pos++ = ch;
			break;
		}
	}
	*cur_pos = '\0';
	return buffer;
}

char const *unescape_json_pointer(char const *fragment, char *buffer)
{
	assert(fragment);

	char *cur_pos = buffer;
	while (true)
	{
		char ch = *fragment++;
		switch (ch)
		{
		case 0:
			*cur_pos = 0;
			return buffer;
		case '~':
			{
				char ch2 = *fragment++;
				switch (ch2)
				{
				case 0:
					*cur_pos++ = ch;
					*cur_pos = 0;
					return buffer;
				case '0':
					*cur_pos++ = '~';
					break;
				case '1':
					*cur_pos++ = '/';
					break;
				default:
					*cur_pos++ = ch;
					*cur_pos++ = ch2;
					break;
				}
			break;
			}
		default:
			*cur_pos++ = ch;
			break;
		}
	}
}

bool uri_scope_push_uri(UriScope *u, char const *uri)
{
	UriParserStateA state;
	UriUriA a;

	state.uri = &a;
	if (URI_SUCCESS != uriParseUriA(&state, uri))
	{
		uriFreeUriMembersA(&a);
		return false;
	}

	UriUriA *result = g_new0(UriUriA, 1);
	UriUriA *base = uri_scope_get_uri(u);
	if (!base)
	{
		memcpy(result, &a, sizeof(a));

		u->uri_stack = g_slist_prepend(u->uri_stack, result);
		return true;
	}

	if (URI_SUCCESS != uriAddBaseUriA(result, &a, base))
	{
		memcpy(result, &a, sizeof(a));

		u->uri_stack = g_slist_prepend(u->uri_stack, result);
		return true;
	}

	u->uri_stack = g_slist_prepend(u->uri_stack, result);
	uriFreeUriMembersA(&a);
	return true;
}

void uri_scope_pop_uri(UriScope *u)
{
	if (!u->uri_stack)
		return;
	GSList *head = u->uri_stack;
	_free_uri(head->data);
	u->uri_stack = g_slist_next(u->uri_stack);
	g_slist_free_1(head);
}
