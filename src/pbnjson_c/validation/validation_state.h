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

#pragma once

#include "error_code.h"
#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Validator Validator;
typedef struct _ValidationState ValidationState;
typedef struct _UriResolver UriResolver;
typedef struct jvalue *jvalue_ref;

typedef struct _Notification
{
	void (*error_func)(ValidationState *s, ValidationErrorCode error, void *ctxt);
	// If this function is NULL, the default value tracking code isn't activated.
	bool (*default_property_func)(ValidationState *s, char const *key, jvalue_ref value, void *ctxt);
} Notification;

typedef struct _ValidationState
{
	UriResolver *uri_resolver;
	Notification *notify;
	GSList *validator_stack;
	GSList *context_stack;
} ValidationState;

ValidationState *validation_state_new(Validator *validator,
                                      UriResolver *uri_resolver,
                                      Notification *notify);
void validation_state_free(ValidationState *s);

void validation_state_init(ValidationState *s,
                           Validator *validator,
                           UriResolver *uri_resolver,
                           Notification *notify);
void validation_state_clear(ValidationState *s);

Validator *validation_state_get_validator(ValidationState *s);
void validation_state_push_validator(ValidationState *s, Validator *v);
Validator *validation_state_pop_validator(ValidationState *s);

void *validation_state_get_context(ValidationState *s);
void validation_state_set_context(ValidationState *s, void *ctxt);
void validation_state_push_context(ValidationState *s, void *ctxt);
void *validation_state_pop_context(ValidationState *s);
void validation_state_notify_error(ValidationState *s, ValidationErrorCode error, void *ctxt);

bool validation_state_have_default_properties(ValidationState *s);
bool validation_state_issue_default_property(ValidationState *s,
                                             char const *key, jvalue_ref value,
                                             void *ctxt);

#ifdef __cplusplus
}
#endif
