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

/** @brief Notifications from the validation (for instance, error condition or default property). */
typedef struct _Notification
{
	/** @brief Error callback
	 *
	 * @param[in] s Validation state — validation instance, in which an error occured.
	 * @param[in] error Validation error code
	 * @param[in] ctxt User-supplied pointer, see validation_check().
	 */
	void (*error_func)(ValidationState *s, ValidationErrorCode error, void *ctxt);

	/** @brief Notification about default property activation.
	 *
	 * If this function is NULL, the default value tracking code isn't activated.
	 * @param[in] s Validation state
	 * @param[in] key Property name
	 * @param[in] value Property value
	 * @param[in] ctxt User-supplied pointer (see validation_check()). This pointer must somehow
	 *                 track where to insert the default value.
	 */
	bool (*default_property_func)(ValidationState *s, char const *key, jvalue_ref value, void *ctxt);
} Notification;


/** @brief Validation instance class
 *
 * Two stacks are used to validate YAJL event (object start, object end,
 * object key, array start, array end, string, bool etc) against schema:
 * validator stack and context stack. The head of the validator stack contains
 * currently active validator, which check incoming events. A validator may
 * push another one to the stack of validators, thus, deferring
 * its own execution (consider validation of an object's property).
 * If validator is finished either because of failure or because of success,
 * it shall remove itself from the validation stack.
 *
 * The stack of contexts may used by validators to keep their data between
 * different events. For instance, object validator should count seen keys
 * to decide if all required properties have been checked. So a structure
 * with the counter is placed into the context state when validator starts
 * its work (object begin), updated when a new key is considered (object key),
 * checked and removed from the stack when the validator finishes its mission
 * (object end).
 */
typedef struct _ValidationState
{
	UriResolver *uri_resolver;   /** @brief To find target validator for $ref as they're encountered. */
	Notification *notify;        /** @brief To notify errors, default values. */
	GSList *validator_stack;     /** @brief Validators being processed, current on top. */
	GSList *context_stack;       /** @brief Data, which may be stored by validators. */
} ValidationState;


/** @brief Constructor.
 *
 * Allocate and initialize new validation instance.
 * @param[in] validator Root validator that the validation should comply
 * @param[in] uri_resolver Map of the resolved internal and external validators.
 *                         If there're unresolved validators in the tree,
 *                         the validation may fail.
 * @param[in] notify Notification callbacks for error details or default properties.
 * @return Newly allocated and initialized validation instance.
 *         It's assumed this constructor never fails, the program is terminated otherwise.
 */
ValidationState *validation_state_new(Validator *validator,
                                      UriResolver *uri_resolver,
                                      Notification *notify);

/** @brief Destructor. */
void validation_state_free(ValidationState *s);

/** @brief Initialize validation instance given it's already allocated.
 *
 * @param[in] s This object
 * @param[in] validator Root validator that the validation should comply
 * @param[in] uri_resolver Map of the resolved internal and external validators.
 *                         If there're unresolved validators in the tree,
 *                         the validation may fail.
 * @param[in] notify Notification callbacks for error details or default properties.
 */
void validation_state_init(ValidationState *s,
                           Validator *validator,
                           UriResolver *uri_resolver,
                           Notification *notify);

/** @brief Deinitialize validation instance. Counterpart to validation_state_init(). */
void validation_state_clear(ValidationState *s);

/** @brief Get current validator, which is in the top of the stack. */
Validator *validation_state_get_validator(ValidationState *s);

/** @brief Push another validator to the stack of validators.
 *
 * This function is used by the validators to give control
 * to child validators.
 */
void validation_state_push_validator(ValidationState *s, Validator *v);

/** @brief Pop the top validator from the stack.
 *
 * Validators pop themself when an error occurs or they're finished otherwise.
 */
Validator *validation_state_pop_validator(ValidationState *s);

/** @brief Get the current data in the top of the stack.
 *
 * Validators may put their belongings for later use. This may be needed,
 * if few events are consumed for the validation to succeed.
 * Of course, validators themselves are responsible to pop their data
 * from the stack consistently.
 */
void *validation_state_get_context(ValidationState *s);

/** @brief Change the current data in the top of the stack. */
void validation_state_set_context(ValidationState *s, void *ctxt);

/** @brief Push another data into the context stack. */
void validation_state_push_context(ValidationState *s, void *ctxt);

/** @brief Pop data from the context stack. */
void *validation_state_pop_context(ValidationState *s);

/** @brief Engage error callback.
 *
 * @param[in] s This object
 * @param[in] error Validation error code
 * @param[in] ctxt User-supplied context pointer to set in the error callback.
 */
void validation_state_notify_error(ValidationState *s, ValidationErrorCode error, void *ctxt);

/** @brief Check if this validation instance has callback to report default properties.
 *
 * If the function is NULL, the default properties tracking code isn't activated at all.
 * This is useful, for instance, when a JSON instance is validated, not being built.
 */
bool validation_state_have_default_properties(ValidationState *s);

/** @brief Report missed default property.
 *
 * @param[in] s This object
 * @param[in] key Property key
 * @param[in] value Property value
 * @param[in] ctxt User-supplied context pointer.
 */
bool validation_state_issue_default_property(ValidationState *s,
                                             char const *key, jvalue_ref value,
                                             void *ctxt);

#ifdef __cplusplus
}
#endif
