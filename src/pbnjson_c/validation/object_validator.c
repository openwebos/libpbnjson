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

#include "object_validator.h"
#include "validation_state.h"
#include "validation_event.h"
#include "generic_validator.h"
#include "object_properties.h"
#include "object_required.h"
#include "uri_resolver.h"
#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <assert.h>

typedef struct _MyContext
{
	bool has_started;   // Has an object been opened with "{"?
	size_t required_count; // Count of required properties
	size_t properties_count;

	GHashTable *default_properties; // char const * -> jvalue_ref, doesn't own anything.
} MyContext;

static void prepare_default_properties(ObjectValidator *o, ValidationState *s, MyContext *my_ctxt)
{
	if (!o->properties ||
	    !validation_state_have_default_properties(s) ||
	    !o->default_properties_count)
	{
		return;
	}

	GHashTable *defaults = object_properties_gather_default(o->properties, s);
	o->default_properties_count = defaults ? g_hash_table_size(defaults) : 0;
	my_ctxt->default_properties = defaults;
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	ObjectValidator *vobj = (ObjectValidator *) v;
	MyContext *my_ctxt = (MyContext *) validation_state_get_context(s);
	if (!my_ctxt->has_started)
	{
		if (e->type != EV_OBJ_START)
		{
			validation_state_notify_error(s, VEC_NOT_OBJECT, ctxt);
			validation_state_pop_validator(s);
			return false;
		}
		my_ctxt->has_started = true;
		my_ctxt->required_count = 0;
		prepare_default_properties(vobj, s, my_ctxt);
		return true;
	}

	if (e->type == EV_OBJ_END)
	{
		if (vobj->min_properties != -1 && vobj->min_properties > my_ctxt->properties_count)
		{
			validation_state_notify_error(s, VEC_NOT_ENOUGH_KEYS, ctxt);
			validation_state_pop_validator(s);
			return false;
		}

		// If reuquired keys count doesn't match to the seen required keys,
		// that's bad. Please note, that duplicates are handled on the higher
		// level, where a map is formed.
		if (vobj->required &&
		    my_ctxt->required_count != object_required_size(vobj->required))
		{
			validation_state_notify_error(s, VEC_NOT_ENOUGH_KEYS, ctxt);
			validation_state_pop_validator(s);
			return false;
		}

		// Issue the default properties not seen before.
		if (my_ctxt->default_properties && g_hash_table_size(my_ctxt->default_properties))
		{
			assert(validation_state_have_default_properties(s));
			GHashTableIter it;
			g_hash_table_iter_init(&it, my_ctxt->default_properties);
			char const *key = NULL;
			jvalue_ref value = NULL;
			while (g_hash_table_iter_next(&it, (gpointer *) &key, (gpointer *) &value))
			{
				if (!validation_state_issue_default_property(s, key, value, ctxt))
					return false;
			}
		}

		validation_state_pop_validator(s);
		return true;
	}

	//FIXME: Does YAJL do basic layout validation?
	//if (e->type != EV_OBJ_KEY)
	//{
	//	validation_state_pop_validator(s);
	//	return false;
	//}

	char key[e->value.string.len + 1];
	strncpy(key, e->value.string.ptr, e->value.string.len);
	key[e->value.string.len] = 0;

	if (vobj->required &&
	    object_required_lookup_key(vobj->required, key))
	{
		++my_ctxt->required_count;
	}

	++my_ctxt->properties_count;

	if (vobj->max_properties != -1 && my_ctxt->properties_count > vobj->max_properties)
	{
		validation_state_notify_error(s, VEC_TOO_MANY_KEYS, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	// Since the key has been seen, don't expect it among defaults.
	if (my_ctxt->default_properties)
	{
		g_hash_table_remove(my_ctxt->default_properties, key);
	}

	// lookup validator by key
	// if not found, use generic validator
	Validator *child = NULL;
	if (vobj->properties)
		child = object_properties_lookup(vobj->properties, key);

	if (child)
	{
		validation_state_push_validator(s, child);
		return true;
	}

	if (!vobj->additional_properties)
	{
		validation_state_notify_error(s, VEC_OBJECT_PROPERTY_NOT_ALLOWED, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	validation_state_push_validator(s, vobj->additional_properties);
	return true;
}

static bool _init_state(Validator *v, ValidationState *s)
{
	MyContext *my_ctxt = g_slice_new0(MyContext);
	my_ctxt->has_started = false;
	validation_state_push_context(s, my_ctxt);
	return true;
}

static void _cleanup_state(Validator *v, ValidationState *s)
{
	MyContext *my_ctxt = validation_state_pop_context(s);
	if (my_ctxt->default_properties)
		g_hash_table_destroy(my_ctxt->default_properties);
	g_slice_free(MyContext, my_ctxt);
}

static void _release(Validator *validator)
{
	ObjectValidator *v = (ObjectValidator *) validator;
	object_validator_release(v);
}

static void _set_properties(Validator *v, ObjectProperties *p)
{
	ObjectValidator *o = (ObjectValidator *) v;
	if (o->properties)
		object_properties_unref(o->properties);
	o->properties = object_properties_ref(p);
}

static void _set_additional_properties(Validator *v, Validator *additional)
{
	ObjectValidator *o = (ObjectValidator *) v;
	if (o->additional_properties)
		validator_unref(o->additional_properties);
	o->additional_properties = validator_ref(additional);
}

static void _set_required(Validator *v, ObjectRequired *p)
{
	ObjectValidator *o = (ObjectValidator *) v;
	if (o->required)
		object_required_unref(o->required);
	o->required = object_required_ref(p);
}

static void _set_max_properties(Validator *v, size_t max)
{
	ObjectValidator *o = (ObjectValidator *) v;
	object_validator_set_max_properties(o, max);
}

static void _set_min_properties(Validator *v, size_t min)
{
	ObjectValidator *o = (ObjectValidator *) v;
	object_validator_set_min_properties(o, min);
}

static void _visit(Validator *v,
                   VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                   void *ctxt)
{
	ObjectValidator *o = (ObjectValidator *) v;
	if (o->additional_properties)
	{
		enter_func(NULL, o->additional_properties, ctxt);
		validator_visit(o->additional_properties, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, o->additional_properties, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(o->additional_properties);
			o->additional_properties = new_v;
		}
	}
	if (o->properties)
		object_properties_visit(o->properties, enter_func, exit_func, ctxt);
}

static void _dump_enter(char const *key, Validator *v, void *ctxt)
{
	//ObjectValidator *o = (ObjectValidator *) v;
	fprintf((FILE *) ctxt, "{");
}

static void _dump_exit(char const *key, Validator *v, void *ctxt, Validator **new_v)
{
	//ObjectValidator *o = (ObjectValidator *) v;
	fprintf((FILE *) ctxt, "}");
}

ValidatorVtable object_vtable =
{
	.check = _check,
	.init_state = _init_state,
	.cleanup_state = _cleanup_state,
	.release = _release,
	.set_object_properties = _set_properties,
	.set_object_additional_properties = _set_additional_properties,
	.set_object_required = _set_required,
	.set_object_max_properties = _set_max_properties,
	.set_object_min_properties = _set_min_properties,
	.visit = _visit,
	.dump_enter = _dump_enter,
	.dump_exit = _dump_exit,
};

ObjectValidator* object_validator_new(void)
{
	ObjectValidator *self = g_new0(ObjectValidator, 1);
	if (!self)
		return NULL;
	self->max_properties = -1;
	self->min_properties = -1;
	self->default_properties_count = -1;
	validator_init(&self->base, &object_vtable);
	self->additional_properties = validator_ref(GENERIC_VALIDATOR);
	return self;
}

void object_validator_release(ObjectValidator *v)
{
	object_properties_unref(v->properties);
	validator_unref(v->additional_properties);
	object_required_unref(v->required);
	g_free(v);
}

bool object_validator_set_max_properties(ObjectValidator *o, size_t max)
{
	o->max_properties = max;
	return true;
}

bool object_validator_set_min_properties(ObjectValidator *o, size_t min)
{
	o->min_properties = min;
	return true;
}
