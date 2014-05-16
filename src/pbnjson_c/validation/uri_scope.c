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
	g_slist_free_full(u->fragment_stack, g_free);
	g_free(u);
}

UriUriA *uri_scope_get_uri(UriScope const *u)
{
	if (!u->uri_stack)
		return NULL;
	return (UriUriA *) u->uri_stack->data;
}

char const *uri_scope_get_fragment(UriScope const *u)
{
	if (!u->fragment_stack)
		return NULL;
	return (char const *) u->fragment_stack->data;
}

char *uri_scope_steal_fragment(UriScope *u)
{
	if (!u->fragment_stack)
		return NULL;
	char *result = (char *) u->fragment_stack->data;
	u->fragment_stack->data = NULL;
	return result;
}

int uri_scope_get_document_length(UriScope const *u)
{
	UriUriA *uri = uri_scope_get_uri(u);
	int result = 0;
	if (URI_SUCCESS != uriToStringCharsRequiredA(uri, &result))
		return -1;
	return result + 1;
}

char const *uri_scope_get_document(UriScope const *u, char *buffer, int chars_required)
{
	UriUriA *uri = uri_scope_get_uri(u);
	if (URI_SUCCESS != uriToStringA(buffer, uri, chars_required, NULL))
		return NULL;
	return buffer;
}

char const *escape_json_pointer(char const *fragment, char *buffer)
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

static char const *uri_scope_push_fragment(UriScope *u, char const *fragment)
{
	char const *hash = strchr(fragment, '#');
	fragment = hash ? hash : "#";

	u->fragment_stack = g_slist_prepend(u->fragment_stack, g_strdup(fragment));
	return uri_scope_get_fragment(u);
}

static char const *uri_scope_pop_fragment(UriScope *u)
{
	if (!u->fragment_stack)
		return NULL;
	g_free(u->fragment_stack->data);
	GSList *head = u->fragment_stack;
	u->fragment_stack = g_slist_next(u->fragment_stack);
	g_slist_free_1(head);
	return uri_scope_get_fragment(u);
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

		uri_scope_push_fragment(u, result->fragment.first ? result->fragment.first - 1 : ROOT_FRAGMENT);
		result->fragment.first = NULL;
		result->fragment.afterLast = NULL;

		u->uri_stack = g_slist_prepend(u->uri_stack, result);
		return true;
	}

	if (URI_SUCCESS != uriAddBaseUriA(result, &a, base))
	{
		memcpy(result, &a, sizeof(a));

		uri_scope_push_fragment(u, result->fragment.first ? result->fragment.first - 1 : ROOT_FRAGMENT);
		result->fragment.first = NULL;
		result->fragment.afterLast = NULL;

		u->uri_stack = g_slist_prepend(u->uri_stack, result);
		return true;
	}

	uri_scope_push_fragment(u, result->fragment.first ? result->fragment.first - 1 : ROOT_FRAGMENT);
	result->fragment.first = NULL;
	result->fragment.afterLast = NULL;

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
	uri_scope_pop_fragment(u);
}

char const *uri_scope_push_fragment_leaf(UriScope *u, char const *leaf)
{
	assert(u->fragment_stack && u->fragment_stack->data);
	char *f = (char *) u->fragment_stack->data;
	assert(f);

	while (leaf[0] == '/')
		++leaf;

	char *new_f = g_strconcat(f, "/", leaf, NULL);
	u->fragment_stack->data = new_f;
	g_free(f);
	return uri_scope_get_fragment(u);
}

char const *uri_scope_pop_fragment_leaf(UriScope *u)
{
	assert(u->fragment_stack && u->fragment_stack->data);
	char const *fragment = uri_scope_get_fragment(u);
	char *slash = strrchr(fragment, '/');
	assert(slash);
	*slash = 0;
	return uri_scope_get_fragment(u);
}
