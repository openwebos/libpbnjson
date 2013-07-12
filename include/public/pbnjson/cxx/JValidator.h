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


#ifndef JVALIDATOR_H_
#define JVALIDATOR_H_

#include "japi.h"
#include "JValue.h"
#include "JSchema.h"
#include "JResolver.h"

namespace pbnjson {

class JValidator {
public:

	/**
	 * Check validity of JValue against JSchema. The function is able to solve external references. In order to check
	 * schema with externals, provide a propriate resolver.
	 *
	 * @param jValue A reference to the JSON object to check
	 * @param jSchema A reference to schema
	 * @param jResolver A reference to resolver of externals
	 * @return true if val is valid against schema
	 *
	 */
	static bool isValid(const JValue &jValue, const JSchema &jSchema, JResolver &jResolver);

	/**
	 * Check validity of JValue against JSchema. The function is not able to resolve externals
	 *
	 * @param jValue A reference to the JSON object to check
	 * @param jSchema A reference to schema
	 * @return true if val is valid against schema
	 *
	 */
	static bool isValid(const JValue &jValue, const JSchema &jSchema);
};

}


#endif // JVALIDATOR_H_
