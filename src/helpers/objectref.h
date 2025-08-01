/*
    SPDX-FileCopyrightText: 2010-2025 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OBJECTREF_H
#define OBJECTREF_H

#include <cstdlib>
#include <limits>
#include <vector>

#include <QtGlobal>

namespace SubtitleComposer {

template<class T>
struct ObjectRefAllocator {
	typedef T value_type;

	ObjectRefAllocator() = default;

	template<class U>
	constexpr ObjectRefAllocator(const ObjectRefAllocator<U> &) noexcept {}

	[[nodiscard]] T * allocate(std::size_t n) {
#ifdef __cpp_exceptions
		if(n > std::numeric_limits<std::size_t>::max() / sizeof(T))
			throw std::bad_array_new_length();
#else
		Q_ASSERT(n <= std::numeric_limits<std::size_t>::max() / sizeof(T));
#endif

		if(T *p = static_cast<T *>(std::malloc(n * sizeof(T)))) {
			lastData = p;
			lastSize = n;
			return p;
		}

#ifdef __cpp_exceptions
		throw std::bad_alloc();
#else
		Q_ASSERT(false);
#endif
	}

	void deallocate(T *p, std::size_t n) noexcept {
		Q_UNUSED(n);
		std::free(p);
	}

private:
	friend T;
	T *lastData = nullptr;
	std::size_t lastSize = 0;
};

template<class T>
class ObjectRef;
template<class T>
using ObjectRefArray = std::vector<ObjectRef<T>, ObjectRefAllocator<ObjectRef<T>>>;

template<class T>
class ObjectRef {
	// Class T must have a ObjectRef<T> *m_ref and implement "const ObjectRefArray<T> * T::refContainer();".
	// T::refContainer() will return "ObjectRefArray<T> *" in which we are supposed to be. We will set *m_ref to our location
	// in container. That way we can know the index in std::vector while holding a pointer without iterating through all
	// elements.
	friend T;

public:
	// default ctor
	ObjectRef() : m_obj(nullptr) { }

	// param ctor
	ObjectRef(T *obj) : m_obj(obj) { ref(); }

	// copy ctor/oper
	ObjectRef(const ObjectRef<T> &other) : m_obj(other.m_obj) { ref(); }
	ObjectRef & operator=(const ObjectRef<T> &other) { m_obj = other.m_obj, ref(); return *this; }

	// move ctor/oper
	ObjectRef(ObjectRef<T> &&other) noexcept : m_obj(other.m_obj) { moveref(&other); }
	ObjectRef & operator=(ObjectRef<T> &&other) noexcept { m_obj = other.m_obj; moveref(&other); return *this; }

private:
	inline void ref()
	{
		if(m_obj && inContainer())
			m_obj->m_ref = this;
	}

	inline void moveref(ObjectRef *other)
	{
		Q_ASSERT(m_obj != nullptr);
		other->m_obj = nullptr;
		if(m_obj->m_ref == other || inContainer())
			m_obj->m_ref = this;
	}

	inline const ObjectRefArray<T> * container()
	{
		Q_ASSERT(m_obj != nullptr);
		const ObjectRefArray<T> *container = m_obj->refContainer();
		Q_ASSERT(container != nullptr);
		return container;
	}

	inline bool inContainer()
	{
		const auto &a = container()->get_allocator();
		return this >= a.lastData && this < a.lastData + a.lastSize;
	}

public:
	// helpers
	inline T * operator->() const { Q_ASSERT(m_obj != nullptr); return m_obj; }
	inline operator T & () const { Q_ASSERT(m_obj != nullptr); return *m_obj; }
	inline operator T * () const { return m_obj; }
	inline T * obj() const { return m_obj; }

private:
	T *m_obj;
};

}

template<class T, class U>
bool operator==(const SubtitleComposer::ObjectRefAllocator<T> &a, const SubtitleComposer::ObjectRefAllocator<U> &b) { return a == b; }

template<class T, class U>
bool operator!=(const SubtitleComposer::ObjectRefAllocator<T> &a, const SubtitleComposer::ObjectRefAllocator<U> &b) { return !operator==(a, b); }

#endif // OBJECTREF_H
