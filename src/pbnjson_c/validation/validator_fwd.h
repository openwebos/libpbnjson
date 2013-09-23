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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Validator Validator;

/** @brief Validator visitor enter function.
 *
 * @param[in] key Key in the parent validator (property name, definition name etc)
 * @param[in] v Current validator
 * @param[in] ctxt Pointer supplied by the user
 */
typedef void (*VisitorEnterFunc)(char const *key, Validator *v, void *ctxt);

/** @brief Validator visitor exit function.
 *
 * @param[in] key Key in the parent validator (property name, definition name etc)
 * @param[in] v Current validator
 * @param[in] ctxt Pointer supplied by the user
 * @param[out] new_v If not NULL, substite the current validator in parent container.
 */
typedef void (*VisitorExitFunc)(char const *key, Validator *v, void *ctxt, Validator **new_v);

#ifdef __cplusplus
}
#endif
