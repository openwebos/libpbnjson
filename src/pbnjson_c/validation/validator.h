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

#include "validator_fwd.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ValidationEvent ValidationEvent;
typedef struct _ValidationState ValidationState;
typedef struct _ObjectProperties ObjectProperties;
typedef struct _ObjectAdditionalProperties ObjectAdditionalProperties;
typedef struct _ObjectRequired ObjectRequired;
typedef struct _ArrayItems ArrayItems;
typedef struct _UriResolver UriResolver;
typedef struct _Number Number;
typedef struct jvalue* jvalue_ref;


/**
 * Table of virtual functions of Validator
 */
typedef struct _ValidatorVtable
{
	/** @brief Increment reference count of the validator */
	Validator* (*ref)(Validator *v);

	/** @brief Decrement reference count of validator. Release validator if reference count drops to zero.
	 *
	 * On release the validator then should free all the resources allocated during construction.
	 */
	void (*unref)(Validator *v);

	/** @name Functions used during validation
	 *  @{
	 */

	/** @brief Check if the given event fits this validator.
	 *
	 * The validator may change its state: push another validator to the stack,
	 * issue an error or remove itself from the stack.
	 */
	bool (*check)(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt);

	/** @brief Init data needed for the validation.
	 *
	 * The ValidationState calls this function to notify the validator
	 * that it's about to be used for validation. The validator can create
	 * data needed for the work here.
	 */
	bool (*init_state)(Validator *v, ValidationState *s);

	/** @brief Clean up the data after validation.
	 *
	 * The ValidationState calls this function to notify the validator
	 * that it's being popped up from the stack. The validator should
	 * clean up what was created after init_state().
	 */
	void (*cleanup_state)(Validator *v, ValidationState *s);

	/** @brief The validator gets reactivated in the stack.
	 *
	 * The ValidationState calls this function to notify the validator
	 * that it's become the head of the stack again. This can happen
	 * if the validator pushed another one into the stack in the past.
	 */
	void (*reactivate)(Validator *v, ValidationState *s);

	/** @} */


	/** @name Post-parse processing of the validator tree
	 *  @{
	 */

	/** @brief Descend to children and execute the function (not recursively)
	 *
	 * This functions should be defined in those validators, which are containers.
	 * When it's called, the validator should call it for its children.
	 * Thus, DFS is implemented.
	 */
	void (*visit)(Validator *v,
	              VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
	              void *ctxt);

	/** @brief Move parsed features into validators.
	 *
	 * This function traverses the tree using #visit, moving gathered features
	 * into corresponding validators.
	 */
	void (*apply_features)(char const *key, Validator *v, void *ctxt);

	/** @brief Combine type validator with combaining validators.
	 *
	 * This function traverses the tree using #visit, and finalizes creation of combining validators
	 * (allOf, anyOf, etc.)
	 */
	void (*combine_validators)(char const *key, Validator *v, void *ctxt, Validator **new_v);

	/** @brief Post process the tree.
	 *
	 * Traverse the tree using #visit, and substitute SchemaParsing with its type_validator.
	 */
	void (*finalize_parse)(char const *key, Validator *v, void *ctxt, Validator **new_v);

	/** @brief Track URI scope when traversing the tree.
	 *
	 * Track URI scope change induced by "id": remember validators under #/definitions.
	 * This function pushes resolved (against parent context) URI scope into URI scope stack.
	 */
	void (*collect_uri_enter)(char const *key, Validator *v, void *ctxt);

	/** @brief Return to previous URI scope. */
	void (*collect_uri_exit)(char const *key, Validator *v, void *ctxt, Validator **new_v);

	/** \brief Dump validator for debugging purposes. */
	void (*dump_enter)(char const *key, Validator *v, void *ctxt);
	/** \brief Finish dumping validator for debugging purposes. */
	void (*dump_exit)(char const *key, Validator *v, void *ctxt, Validator **new_v);

	/** @} */

	/** @name Apply validator features
	 *  @{
	 */

	Validator* (*set_object_properties)(Validator *v, ObjectProperties *p);
	Validator* (*set_object_additional_properties)(Validator *v, Validator *additional);
	Validator* (*set_object_required)(Validator *v, ObjectRequired *p);
	Validator* (*set_object_max_properties)(Validator *v, size_t max);
	Validator* (*set_object_min_properties)(Validator *v, size_t min);
	Validator* (*set_array_items)(Validator *v, ArrayItems *a);
	Validator* (*set_array_additional_items)(Validator *v, Validator *additional);
	Validator* (*set_array_max_items)(Validator *v, size_t maxItems);
	Validator* (*set_array_min_items)(Validator *v, size_t minItems);
	Validator* (*set_number_maximum)(Validator *v, Number *n);
	Validator* (*set_number_maximum_exclusive)(Validator *v, bool exclusive);
	Validator* (*set_number_minimum)(Validator *v, Number *n);
	Validator* (*set_number_minimum_exclusive)(Validator *v, bool exclusive);
	Validator* (*set_string_max_length)(Validator *v, size_t maxLength);
	Validator* (*set_string_min_length)(Validator *v, size_t minLength);
	Validator* (*set_default)(Validator *v, jvalue_ref def_value);
	jvalue_ref (*get_default)(Validator *v, ValidationState *s);

	/** @} */
} ValidatorVtable;

/** Base structure of validator */
typedef struct _Validator
{
	ValidatorVtable *vtable;    /**< @brief Table of virtual functions */
} Validator;

/** @name Base functions
 *  @{
 */

/** @brief Initialize fields of an already allocated validator.
 *
 * @param[in] v Validator to initialize
 * @param[in] vtable Pointer to the table of virtual functions to set.
 */
void validator_init(Validator *v, ValidatorVtable *vtable);

/** @brief Increment reference count of the validator */
Validator* validator_ref(Validator *v);

/** @brief Decrement reference count of the validator */
void validator_unref(Validator *v);

/** @} */


/** @name Virtual functions of validator
 *  @{
 */

/** @brief Validate an event.
 *
 * Check if a given event fits with the current validator. The validation state
 * may change consequently: either the validator pops itself from the stack,
 * or pushes another one to check expected further events.
 * @param[in] v This validator
 * @param[in] e Event to validate
 * @param[in] s Validation state
 * @param[in] ctxt Event context, which will be used for error notifications.
 * @return false if validation failed, and it's pointless to continue; true if
 *         validation succeeded so far.
 */
bool validator_check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt);

bool validator_init_state(Validator *v, ValidationState *s);
void validator_cleanup_state(Validator *v, ValidationState *s);
void validator_reactivate(Validator *v, ValidationState *s);

/** @brief Visit validator and its descendants.
 *
 * Call enter_func and exit_func for every contained (descendant) validator of this one.
 * This function effectively implements DFS.
 * @param[in] v This validator.
 * @param[in] enter_func Callback function when a validator is entered.
 * @param[in] exit_func Callback function when a validator is left.
 * @param[in] ctxt The context to use when calling the callbacks.
 */
void validator_visit(Validator *v,
                     VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                     void *ctxt);

void VISITOR_ENTER_VOID(char const *key, Validator *v, void *ctxt);
void VISITOR_EXIT_VOID(char const *key, Validator *v, void *ctxt, Validator **new_v);

void _validator_apply_features(char const *key, Validator *v, void *ctxt);
void validator_apply_features(Validator *v);

void _validator_combine(char const *key, Validator *v, void *ctxt, Validator **new_v);
void validator_combine(Validator *v);

void _validator_finalize_parse(char const *key, Validator *v, void *ctxt, Validator **new_v);
Validator* validator_finalize_parse(Validator *v);

void _validator_collect_uri_enter(char const *key, Validator *v, void *ctxt);
void _validator_collect_uri_exit(char const *key, Validator *v, void *ctxt, Validator **new_v);
void validator_collect_uri(Validator *v, char const *document, UriResolver *u);

void _validator_dump_enter(char const *key, Validator *v, void *ctxt);
void _validator_dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v);
void validator_dump(Validator *v, FILE *f);

Validator* validator_set_object_properties(Validator *v, ObjectProperties *p);
Validator* validator_set_object_additional_properties(Validator *v, Validator *additional);
Validator* validator_set_object_required(Validator *v, ObjectRequired *p);
Validator* validator_set_object_max_properties(Validator *v, size_t max);
Validator* validator_set_object_min_properties(Validator *v, size_t min);
Validator* validator_set_array_items(Validator *v, ArrayItems *a);
Validator* validator_set_array_additional_items(Validator *v, Validator *additional);
Validator* validator_set_array_max_items(Validator *v, size_t maxItems);
Validator* validator_set_array_min_items(Validator *v, size_t minItems);
Validator* validator_set_number_maximum(Validator *v, Number *n);
Validator* validator_set_number_maximum_exclusive(Validator *v, bool exclusive);
Validator* validator_set_number_minimum(Validator *v, Number *n);
Validator* validator_set_number_minimum_exclusive(Validator *v, bool exclusive);
Validator* validator_set_string_max_length(Validator *v, size_t maxLength);
Validator* validator_set_string_min_length(Validator *v, size_t minLength);
Validator* validator_set_default(Validator *v, jvalue_ref def_value);
jvalue_ref validator_get_default(Validator *v, ValidationState *s);

/** @} */

#ifdef __cplusplus
}
#endif
