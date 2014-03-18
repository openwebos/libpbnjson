// @@@LICENSE
//
//      Copyright (c) 2014 LG Electronics, Inc.
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

#ifndef JTRAVERSE_H_
#define JTRAVERSE_H_

#include <inttypes.h>
#include <jtypes.h>
#include <stdbool.h>

typedef struct TraverseCallbacks {
	bool (*jnull)(void *ctxt);
	bool (*jbool)(void *ctxt, bool value);
	bool (*jnumber_int)(void *ctxt, int64_t num);
	bool (*jnumber_double)(void *ctxt, double num);
	bool (*jnumber_raw)(void *ctxt, const char *num, size_t len);
	bool (*jstring)(void *ctxt, const unsigned char *str, size_t len);
	bool (*jobj_start)(void *ctxt);
	bool (*jobj_key)(void *ctxt, const unsigned char *key, size_t len);
	bool (*jobj_end)(void *ctxt);
	bool (*jarr_start)(void *ctxt);
	bool (*jarr_end)(void *ctxt);
	void (*jarray)(void *ctxt, jvalue_ref jref);
} *TraverseCallbacksRef;

bool jvalue_traverse(jvalue_ref jref, TraverseCallbacksRef tc, void *context);

#endif /* JTRAVERSE_H_ */
