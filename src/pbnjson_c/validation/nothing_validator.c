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

#include "nothing_validator.h"

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	// Depressive validator, never succeeds.
	return false;
}

static ValidatorVtable nothing_vtable =
{
	.check = _check,
};

Validator NOTHING_VALIDATOR_IMPL =
{
	.ref_count = 1,
	.vtable = &nothing_vtable,
};

Validator *NOTHING_VALIDATOR = &NOTHING_VALIDATOR_IMPL;
