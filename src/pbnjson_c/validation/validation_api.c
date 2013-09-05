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

#include "validation_api.h"
#include "../yajl_compat.h"
#include <yajl/yajl_parse.h>
#include <stdio.h>
#include <string.h>

bool validation_check(ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	Validator *v = validation_state_get_validator(s);
	if (!v)
		return false;
	return validator_check(v, e, s, ctxt);
}

/////////////////////////////////////////////////////////////////////////////////

typedef struct _ValidationCtxt
{
	Notification notify;
	ValidationState *s;
	yajl_handle yh;
	ValidationError *error;
	bool error_set;
} ValidationCtxt;

static int on_null(void *ctx)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_null();
	return validation_check(&e, vc->s, ctx);
}

static int on_boolean(void *ctx, int boolean)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_boolean(boolean);
	return validation_check(&e, vc->s, ctx);
}

static int on_number(void *ctx, const char *str, yajl_size_t len)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_number(str, len);
	return validation_check(&e, vc->s, ctx);
}

static int on_string(void *ctx, const unsigned char *str, yajl_size_t len)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_string((char const *) str, len);
	return validation_check(&e, vc->s, ctx);
}

static int on_map_key(void *ctx, const unsigned char *str, yajl_size_t len)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_obj_key((char const *) str, len);
	return validation_check(&e, vc->s, ctx);
}

static int on_start_map(void *ctx)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_obj_start();
	return validation_check(&e, vc->s, ctx);
}

static int on_end_map(void *ctx)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_obj_end();
	return validation_check(&e, vc->s, ctx);
}

static int on_start_array(void *ctx)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_arr_start();
	return validation_check(&e, vc->s, ctx);
}

static int on_end_array(void *ctx)
{
	ValidationCtxt *vc = (ValidationCtxt *) ctx;
	ValidationEvent e = validation_event_arr_end();
	return validation_check(&e, vc->s, ctx);
}

static yajl_callbacks callbacks =
{
	.yajl_null          = on_null,
	.yajl_boolean       = on_boolean,
	.yajl_integer       = NULL,
	.yajl_double        = NULL,
	.yajl_number        = on_number,
	.yajl_string        = on_string,
	.yajl_start_map     = on_start_map,
	.yajl_map_key       = on_map_key,
	.yajl_end_map       = on_end_map,
	.yajl_start_array   = on_start_array,
	.yajl_end_array     = on_end_array
};

static void _on_error(ValidationState *s, ValidationErrorCode error, void *ctxt)
{
	ValidationCtxt *validation_ctxt = (ValidationCtxt *) ctxt;
	if (!validation_ctxt /*|| validation_ctxt->error_set*/)
		return;
	ValidationError *ve = validation_ctxt->error;
	if (ve)
	{
		ve->error = error;
		ve->offset = yajl_get_bytes_consumed(validation_ctxt->yh);
		validation_ctxt->error_set = true;
	}
}

bool validate_json_n(char const *json, size_t json_len, Validator *v,
                     UriResolver *u, ValidationError *error)
{
	ValidationCtxt ctxt =
	{
		.notify = {
			.error_func = &_on_error,
		},
		.error = error,
		.error_set = false,
	};

	ctxt.s = validation_state_new(v, u, &ctxt.notify);
	if (!ctxt.s)
		return false;

	const bool allow_comments = true;

#if YAJL_VERSION < 20000
	yajl_parser_config yajl_opts =
	{
		allow_comments,
		0,
	};
	ctxt.yh = yajl_alloc(&callbacks, &yajl_opts, NULL, &ctxt);
#else
	ctxt.yh = yajl_alloc(&callbacks, NULL, &ctxt);
	if (ctxt.yh)
		yajl_config(ctxt.yh, yajl_allow_comments, allow_comments ? 1 : 0);
#endif // YAJL_VERSION

	if (!ctxt.yh)
	{
		validation_state_free(ctxt.s);
		return false;
	}

	yajl_status result = yajl_parse(ctxt.yh, (const unsigned char *) json, json_len);
	if (yajl_status_ok != result
#if YAJL_VERSION < 20000
	    && yajl_status_insufficient_data != result
#endif
	    )
	{
		if (error && !ctxt.error_set)
		{
			error->error = VEC_SYNTAX;
			error->offset = yajl_get_bytes_consumed(ctxt.yh);
			ctxt.error_set = true;
		}
		yajl_free(ctxt.yh);
		validation_state_free(ctxt.s);
		return NULL;
	}

#if YAJL_VERSION < 20000
	if (yajl_status_ok != yajl_parse_complete(ctxt.yh))
#else
	if (yajl_status_ok != yajl_complete_parse(ctxt.yh))
#endif // YAJL_VERSION
	{
		if (error && !ctxt.error_set)
		{
			error->error = VEC_SYNTAX;
			error->offset = yajl_get_bytes_consumed(ctxt.yh);
			ctxt.error_set = true;
		}
		yajl_free(ctxt.yh);
		validation_state_free(ctxt.s);
		return NULL;
	}

	yajl_free(ctxt.yh);
	validation_state_free(ctxt.s);
	return true;
}

bool validate_json(char const *json, Validator *v, UriResolver *u, ValidationError *error)
{
	return validate_json_n(json, strlen(json), v, u, error);
}

bool validate_json_plain(char const *json, Validator *v)
{
	return validate_json(json, v, NULL, NULL);
}
