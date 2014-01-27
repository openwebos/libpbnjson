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

#include "pattern.h"
#include "validator.h"
#include <assert.h>

static void release(Feature *f)
{
	Pattern *p = (Pattern *) f;
	if (p->regex)
		g_regex_unref(p->regex);
	g_free(p);
}

static Validator* apply(Feature *f, Validator *v)
{
	assert(f);
	return validator_set_string_pattern(v, (Pattern *) f);
}

static FeatureVtable pattern_vtable =
{
	.release = release,
	.apply = apply,
};

Pattern* pattern_new(void)
{
	Pattern *p = g_new0(Pattern, 1);
	feature_init(&p->base, &pattern_vtable);
	return p;
}

bool pattern_set_regex(Pattern *p, char const *str)
{
	if (p->regex)
		g_regex_unref(p->regex);
	p->regex = g_regex_new(str, 0, 0, NULL);
	return p->regex;
}

bool pattern_set_regex_n(Pattern *p, char const *str, size_t str_len)
{
	char buf[str_len + 1];
	memcpy(buf, str, str_len);
	buf[str_len] = 0;
	bool res = pattern_set_regex(p, buf);
	return res;
}
