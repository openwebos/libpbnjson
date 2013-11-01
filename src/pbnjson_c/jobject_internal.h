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

#ifndef JOBJECT_INTERNAL_H_
#define JOBJECT_INTERNAL_H_

#include <japi.h>
#include <jtypes.h>
#include <glib.h>

#define ARRAY_BUCKET_SIZE (1 << 4)
#define OUTSIDE_ARR_BUCKET_RANGE(value) ((value) & (~(ARRAY_BUCKET_SIZE - 1)))


typedef enum {
	JV_NULL = 0,
	JV_BOOL,
	JV_NUM,
	JV_STR,
	JV_ARRAY,
	JV_OBJECT,
} JValueType;

struct jvalue {
	JValueType m_type;
	ssize_t m_refCnt;
	char *m_toString;
	jdeallocator m_toStringDealloc;
	raw_buffer m_backingBuffer;
	bool m_backingBufferMMap;
};

typedef struct PJSON_LOCAL jvalue jvalue;

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	bool value;
} jbool;

_Static_assert(offsetof(jbool, m_value) == 0, "jbool and jbool.m_value should have the same addresses");

typedef enum {
	NUM_RAW,
	NUM_FLOAT,
	NUM_INT,
} JNumType;

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	union {
		raw_buffer raw;
		double floating;
		int64_t integer;
	} value;
	JNumType m_type;
	ConversionResultFlags m_error;
	jdeallocator m_rawDealloc;
} jnum;

_Static_assert(offsetof(jnum, m_value) == 0, "jnum and jnum.m_value should have the same addresses");

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	jdeallocator m_dealloc;
	raw_buffer m_data;
} jstring;

_Static_assert(offsetof(jstring, m_value) == 0, "jstring and jstring.m_value should have the same addresses");

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	jvalue_ref m_smallBucket[ARRAY_BUCKET_SIZE];
	jvalue_ref *m_bigBucket;
	ssize_t m_size;
	ssize_t m_capacity;
} jarray;

_Static_assert(offsetof(jarray, m_value) == 0, "jarray and jarray.m_value should have the same addresses");

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	GHashTable *m_members;
} jobject;

_Static_assert(offsetof(jobject, m_value) == 0, "jobject and jobject.m_value should have the same addresses");

extern PJSON_LOCAL jvalue JNULL;

PJSON_LOCAL bool jobject_init(jobject *obj);

extern PJSON_LOCAL int64_t jnumber_deref_i64(jvalue_ref num);

extern PJSON_LOCAL bool jboolean_deref_to_value(jvalue_ref boolean);

extern PJSON_LOCAL bool jbuffer_equal(raw_buffer buffer1, raw_buffer buffer2);

extern PJSON_LOCAL raw_buffer jnumber_deref_raw(jvalue_ref num);

extern PJSON_LOCAL bool jarray_has_duplicates(jvalue_ref arr);

inline static jbool* jboolean_deref(jvalue_ref boolean) { return (jbool*)boolean; }

inline static jnum* jnum_deref(jvalue_ref num) { return (jnum*)num; }

inline static jstring* jstring_deref(jvalue_ref str) { return (jstring*)str; }

inline static jarray* jarray_deref(jvalue_ref array) { return (jarray*)array; }

inline static jobject* jobject_deref(jvalue_ref array) { return (jobject*)array; }

#endif /* JOBJECT_INTERNAL_H_ */
