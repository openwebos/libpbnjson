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

#include <JValue.h>

#include <pbnjson.h>
#include <JSchema.h>
#include <JGenerator.h>
#include <cassert>

#ifdef DBG_CXX_MEM_STR
#define PJ_DBG_CXX_STR(expr) expr
#else
#define PJ_DBG_CXX_STR(expr) do { } while(0)
#endif

namespace pbnjson {

JValue JValue::JNULL(jnull());

static inline raw_buffer strToRawBuffer(const std::string& str)
{
	return (raw_buffer){str.c_str(), str.length()};
}

JValue::JValue()
#ifdef _DEBUG
	: m_jval(jvalue_copy(JNULL.m_jval))
#else
	: m_jval(JNULL.m_jval)
#endif
{
}

JValue::JValue(jvalue_ref toOwn)
	: m_jval(toOwn)
{
	if (toOwn == NULL)
		m_jval = JNULL.m_jval;
}

JValue::JValue(jvalue_ref parsed, std::string const &input)
	: m_jval(parsed), m_input(input)
{
	// if this assertion doesn't hold, the optimization parameters in parse are
	// invalid.
	assert(input.c_str() == m_input.c_str());
	PJ_DBG_CXX_STR(std::cerr << "Have handle to string at " << (void*)m_input.c_str() << std::endl);
}

template <>
JValue::JValue(const int32_t& value)
	: m_jval(jnumber_create_i64(value))
{
}

template <>
JValue::JValue(const int64_t& value)
	: m_jval(jnumber_create_i64(value))
{
}

template <>
JValue::JValue(const double& value)
	: m_jval(jnumber_create_f64(value))
{
}

template <>
JValue::JValue(const std::string &value)
	: m_input(value)
{
	PJ_DBG_CXX_STR(std::cerr << "Have handle to string at " << (void*)m_input.c_str() << std::endl);
	assert(m_input.c_str() == value.c_str());
#if PBNJSON_ZERO_COPY_STL_STR
	m_jval = jstring_create_nocopy(strToRawBuffer(m_input));
	assert(jstring_get_fast(m_jval).m_str == m_input.c_str());
	assert(jstring_get_fast(m_jval).m_len == m_input.length());
#else
	m_jval = jstring_create_utf8(m_input.c_str(), m_input.size());
#endif
}

JValue::JValue(const char *str)
	: m_input(str)
{
	PJ_DBG_CXX_STR(std::cerr << "Have handle to string at " << (void*)m_input.c_str() << std::endl);
#if PBNJSON_ZERO_COPY_STL_STR
	m_jval = jstring_create_nocopy(strToRawBuffer(m_input));
	assert(jstring_get_fast(m_jval).m_str == m_input.c_str() || m_input.length() == 0);
	assert(jstring_get_fast(m_jval).m_len == m_input.length());
#else
	m_jval = jstring_create_utf8(m_input.c_str(), m_input.size());
#endif
}

template <>
JValue::JValue(const bool& value)
	: m_jval(jboolean_create(value))
{
}

template<>
JValue::JValue(const NumericString& value)
	: m_input(value)
{
#if PBNJSON_ZERO_COPY_STL_STR
	m_jval = jnumber_create_unsafe(strToRawBuffer(m_input), NULL);

#ifdef _DEBUG
	{
		raw_buffer result;
		jnumber_get_raw(m_jval, &result);
		assert(m_input.c_str() == result.m_str);
	}
#endif
#else
	m_jval = jnumber_create(strToRawBuffer(value));
#endif
}

template<>
JValue::JValue(const JValueArrayElement& other)
	: m_jval(jvalue_copy(other.m_jval)), m_input(other.m_input)
#if PBNJSON_ZERO_COPY_STL_STR
	, m_children(other.m_children)
#endif
{

}

JValue::JValue(const JValue& other)
	: m_jval(jvalue_copy(other.m_jval)), m_input(other.m_input)
#if PBNJSON_ZERO_COPY_STL_STR
	, m_children(other.m_children)
#endif
{
}

JValue::~JValue()
{
	PJ_DBG_CXX_STR(std::cerr << "Releasing handle to " << (void *)m_input.c_str() << std::endl);
	j_release(&m_jval);
}


JValue& JValue::operator=(const JValue& other)
{
	if (m_jval != other.m_jval) {
		j_release(&m_jval);
		m_jval = jvalue_copy(other.m_jval);
		m_input = other.m_input;
#if PBNJSON_ZERO_COPY_STL_STR
		m_children = other.m_children;
#endif
	}
	return *this;
}

JValue JValue::duplicate() const
{
	return jvalue_duplicate(this->peekRaw());
}

JValue Object()
{
	return jobject_create();
}

JValue Array()
{
	return jarray_create(NULL);
}

#if 0
template <>
JValue JValue::Value<int64_t>(const int64_t& value)
{
	return JValue(value);
}

template <>
JValue JValue::Value<double>(const double& value)
{
	return jnumber_create_f64(value);
}

template <>
JValue JValue::Value<std::string>(const std::string &value)
{
	// already have the length - why not use it instead of calling strlen one more time
	return JValue (jstring_create_nocopy(strToRawBuffer(value)), value);
}

template<>
JValue JValue::Value<NumericString>(const NumericString& value)
{
	return JValue (jnumber_create_unsafe(strToRawBuffer(static_cast<std::string>(value)), NULL), value);
}

template <>
JValue JValue::Value<bool>(const bool& value)
{
	return jboolean_create(value);
}
#endif

bool JValue::operator==(const JValue& other) const
{
	return jvalue_equal(m_jval, other.m_jval);
}

template <class T>
static bool numEqual(const JValue& jnum, const T& nativeNum)
{
	T num;
	if (jnum.asNumber(num) == CONV_OK)
		return num == nativeNum;
	return false;
}

bool JValue::operator==(const char * other) const
{
	const char * buffer = asCString();
	if (buffer == NULL)
		return false;

	return strcmp(buffer, other) == 0;
}

bool JValue::operator==(const std::string& other) const
{
	const char * buffer = asCString();
	if (buffer == NULL)
		return false;

	return other.compare(buffer) == 0;
}

bool JValue::operator==(const double& other) const
{
	return numEqual(*this, other);
}

bool JValue::operator==(const int64_t& other) const
{
	return numEqual(*this, other);
}

bool JValue::operator==(int32_t other) const
{
	return numEqual(*this, other);
}

bool JValue::operator==(bool other) const
{
	bool value;
	if (asBool(value) == CONV_OK)
		return value == other;
	return false;
}

JValueArrayElement JValue::operator[](int index) const
{
	return JValue(jvalue_copy(jarray_get(m_jval, index)));
}

JValueArrayElement JValue::operator[](const std::string& key) const
{
	return this->operator[](j_str_to_buffer(key.c_str(), key.size()));
}

JValueArrayElement JValue::operator[](const raw_buffer& key) const
{
	return JValueArrayElement(jvalue_copy(jobject_get(m_jval, key)));
}

bool JValue::put(size_t index, const JValue& value)
{
#if PBNJSON_ZERO_COPY_STL_STR
	m_children.push_back(value.m_input);
	m_children.insert(m_children.end(), value.m_children.begin(), value.m_children.end());
#endif
	return jarray_set(m_jval, index, value.peekRaw());
}

bool JValue::put(const std::string& key, const JValue& value)
{
	return put(JValue(key), value);
}

bool JValue::put(const JValue& key, const JValue& value)
{
#if PBNJSON_ZERO_COPY_STL_STR
	m_children.push_back(value.m_input);
	m_children.push_back(key.m_input);
	m_children.insert(m_children.end(), value.m_children.begin(), value.m_children.end());
#endif
	return jobject_set2(m_jval, key.peekRaw(), value.peekRaw());
}

bool JValue::remove(const char *key)
{
	raw_buffer buf;
	buf.m_str = key;
	buf.m_len = strlen(key);
	return jobject_remove(m_jval, buf);
}

bool JValue::remove(const std::string &key)
{
	raw_buffer buf;
	buf.m_str = key.c_str();
	buf.m_len = key.length();
	return jobject_remove(m_jval, buf);
}

bool JValue::remove(const JValue &key)
{
	if (!jis_string(key.m_jval))
		return false;
	return jobject_remove(m_jval, jstring_get_fast(key.m_jval));
}

JValue& JValue::operator<<(const JValue& element)
{
	if (!append(element))
		return Null();
	return *this;
}

JValue& JValue::operator<<(const KeyValue& pair)
{
	if (!put(pair.first, pair.second))
		return Null();
	return *this;
}


bool JValue::append(const JValue& value)
{
#if PBNJSON_ZERO_COPY_STL_STR
	if (!value.m_input.empty() || (value.isString() && value.asString().empty())) {
		m_children.push_back(value.m_input);
	}
	m_children.insert(m_children.end(), value.m_children.begin(), value.m_children.end());
#endif
	return jarray_set(m_jval, jarray_size(m_jval), value.peekRaw());
}

bool JValue::hasKey(const std::string& key) const
{
	return jobject_get_exists(m_jval, strToRawBuffer(key), NULL);
}

ssize_t JValue::objectSize() const
{
	return jobject_size(m_jval);
}

ssize_t JValue::arraySize() const
{
	return jarray_size(m_jval);
}

bool JValue::isNull() const
{
	return jis_null(m_jval);
}

bool JValue::isNumber() const
{
	return jis_number(m_jval);
}

bool JValue::isString() const
{
	return jis_string(m_jval);
}

bool JValue::isObject() const
{
	return jis_object(m_jval);
}

bool JValue::isArray() const
{
	return jis_array(m_jval);
}

bool JValue::isBoolean() const
{
	return jis_boolean(m_jval);
}

template <>
ConversionResultFlags JValue::asNumber<int32_t>(int32_t& number) const
{
	return jnumber_get_i32(m_jval, &number);
}

template <>
ConversionResultFlags JValue::asNumber<int64_t>(int64_t& number) const
{
	return jnumber_get_i64(m_jval, &number);
}

template <>
ConversionResultFlags JValue::asNumber<double>(double& number) const
{
	return jnumber_get_f64(m_jval, &number);
}

//! @cond Doxygen_Suppress
template <>
ConversionResultFlags JValue::asNumber<std::string>(std::string& number) const
{
	raw_buffer asRaw;
	ConversionResultFlags result;

	result = jnumber_get_raw(m_jval, &asRaw);
	number = std::string(asRaw.m_str, asRaw.m_len);

	return result;
}
//! @endcond

template <>
ConversionResultFlags JValue::asNumber<NumericString>(NumericString& number) const
{
	std::string num;
	ConversionResultFlags result;

	result = asNumber(num);
	number = num;

	return result;
}

template <>
int32_t JValue::asNumber<int32_t>() const
{
	int32_t result = 0;
	asNumber(result);
	return result;
}

template <>
int64_t JValue::asNumber<int64_t>() const
{
	int64_t result = 0;
	asNumber(result);
	return result;
}

template <>
double JValue::asNumber<double>() const
{
	double result = 0;
	asNumber(result);
	return result;
}

template <>
std::string JValue::asNumber<std::string>() const
{
	std::string result;
	asNumber(result);
	return result;
}

template <>
NumericString JValue::asNumber<NumericString>() const
{
	return NumericString(asNumber<std::string>());
}

const char * JValue::asCString() const
{
	if (!isString()) {
		return NULL;
	}

	raw_buffer backingBuffer = jstring_get_fast(m_jval);

	return backingBuffer.m_str;
}

ConversionResultFlags JValue::asString(std::string &asStr) const
{
	if (!isString()) {
		return CONV_NOT_A_STRING;
	}

	raw_buffer backingBuffer = jstring_get_fast(m_jval);
	if (backingBuffer.m_str == NULL) {
		asStr = "";
		return CONV_NOT_A_STRING;
	}

	asStr = std::string(backingBuffer.m_str, backingBuffer.m_len);

	return CONV_OK;
}

#if 0
bool JValue::toString(const JSchema& schema, std::string &toStr) const
{
	JGenerator generator;
	return generator.toString(*this, schema, toStr);
}
#endif

ConversionResultFlags JValue::asBool(bool &result) const
{
	return jboolean_get(m_jval, &result);
}

JValue::ObjectIterator::ObjectIterator()
	: _parent(0)
	, _at_end(true)
{
	_key_value.key = 0;
	_key_value.value = 0;
}

JValue::ObjectIterator::ObjectIterator(jvalue_ref parent)
	: _parent(0)
	, _at_end(false)
{
	_key_value.key = 0;
	_key_value.value = 0;

	if (UNLIKELY(!jobject_iter_init(&_it, parent)))
		throw InvalidType("Can't iterate over non-object");

	_parent = jvalue_copy(parent);
	_at_end = !jobject_iter_next(&_it, &_key_value);
}

JValue::ObjectIterator::ObjectIterator(const ObjectIterator& other)
	: _it(other._it)
	, _parent(jvalue_copy(other._parent))
	, _key_value(other._key_value)
	, _at_end(other._at_end)
{
}

JValue::ObjectIterator::~ObjectIterator()
{
	j_release(&_parent);
}

JValue::ObjectIterator& JValue::ObjectIterator::operator=(const ObjectIterator &other)
{
	if (this != &other)
	{
		_it = other._it;
		j_release(&_parent);
		_parent = jvalue_copy(other._parent);
		_key_value = other._key_value;
		_at_end = other._at_end;
	}
	return *this;
}

/**
 * specification says it's undefined, but implementation-wise,
 * the C api will return the current iterator if you try to go past the end.
 *
 */
JValue::ObjectIterator& JValue::ObjectIterator::operator++()
{
	_at_end = !jobject_iter_next(&_it, &_key_value);
	return *this;
}

JValue::ObjectIterator JValue::ObjectIterator::operator++(int)
{
	ObjectIterator result(*this);
	++(*this);
	return result;
}

JValue::ObjectIterator JValue::ObjectIterator::operator+(size_t n) const
{
	ObjectIterator next(*this);
	for (; n > 0; --n)
		++next;
	return next;
}

bool JValue::ObjectIterator::operator==(const ObjectIterator& other) const
{
	if (this == &other)
		return true;
	if (_at_end && other._at_end)
		return true;
	if (_at_end || other._at_end)
		return false;
	return jstring_equal(_key_value.key, other._key_value.key);
}

JValue::KeyValue JValue::ObjectIterator::operator*() const
{
	return KeyValue(jvalue_copy(_key_value.key), jvalue_copy(_key_value.value));
}


/**
 * specification says it's undefined. in the current implementation
 * though, jobj_iter_init should return end() when this isn't an object
 * (it also takes care of printing errors to the log)
 */
JValue::ObjectIterator JValue::begin()
{
	return ObjectIterator(m_jval);
}

/**
 * Specification says it's undefined.  In the current implementation
 * though, jobj_iter_init_last will return a NULL pointer when this isn't
 * an object (it also takes care of printing errors to the log)
 *
 * Specification says undefined if we try to iterate - current implementation
 * won't let you iterate once you hit end.
 */
JValue::ObjectIterator JValue::end()
{
	return ObjectIterator();
}

/**
 * specification says it's undefined. in the current implementation
 * though, jobj_iter_init should return end() when this isn't an object
 * (it also takes care of printing errors to the log)
 */
JValue::ObjectConstIterator JValue::begin() const
{
	return ObjectConstIterator(m_jval);
}

/**
 * Specification says it's undefined.  In the current implementation
 * though, jobj_iter_init_last will return a NULL pointer when this isn't
 * an object (it also takes care of printing errors to the log)
 *
 * Specification says undefined if we try to iterate - current implementation
 * won't let you iterate once you hit end.
 */
JValue::ObjectConstIterator JValue::end() const
{
	return ObjectConstIterator();
}

NumericString::operator JValue()
{
	return JValue(*this);
}

JValueArrayElement::JValueArrayElement(const JValue & value)
	: JValue(value)
{
}

}
