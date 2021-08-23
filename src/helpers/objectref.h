/*
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OBJECTREF_H
#define OBJECTREF_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include <utility>

namespace SubtitleComposer {

template<class T>
class ObjectRef {
	// Class T must have a ObjectRef<T> *m_ref and implement "const QVector<ObjectRef<T>> * T::refContainer();".
	// T::refContainer() will return "QVector *" in which we are supposed to be. We will set *m_ref to our location
	// in container. That way we can know the index in QVector while holding a pointer without iterating through all
	// elements.
	friend T;

public:
	// param ctor/dtor
	ObjectRef(T *obj) : m_obj(obj) { ref(); }
	virtual ~ObjectRef() { unref(); }

	// copy ctor/oper
	ObjectRef(const ObjectRef<T> &other) : m_obj(other.m_obj) { ref(); }
	ObjectRef & operator=(const ObjectRef &other) { unref(); m_obj = other.m_obj, ref(); return *this; }

	// move ctor/oper
	ObjectRef(ObjectRef &&other) noexcept : m_obj(other.m_obj) { moveref(&other); }
	ObjectRef & operator=(ObjectRef &&other) noexcept { unref(); m_obj = other.m_obj; moveref(&other); return *this; }

	// default ctor
	ObjectRef() : m_obj(nullptr) { }

private:
	inline void ref()
	{
		if(m_obj && inContainer())
			m_obj->m_ref = this;
	}

	inline void unref()
	{
//		if(m_obj && m_obj->m_ref == this)
//			m_obj->m_ref = nullptr;
	}

	inline void moveref(ObjectRef *other)
	{
		Q_ASSERT(m_obj != nullptr);
		other->m_obj = nullptr;
		if(m_obj->m_ref == other || inContainer())
			m_obj->m_ref = this;
	}

	inline const QVector<SubtitleComposer::ObjectRef<T>> *container()
	{
		Q_ASSERT(m_obj != nullptr);
		const QVector<SubtitleComposer::ObjectRef<T>> *container = m_obj->refContainer();
		Q_ASSERT(container != nullptr);
		return container;
	}

	inline bool inContainer()
	{
		const QVector<SubtitleComposer::ObjectRef<T>> *vec = container();
		const SubtitleComposer::ObjectRef<T> *data = vec->constData();
		return this >= data && this < data + vec->capacity();
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

// If we declare ObjectRef as Q_MOVABLE_TYPE, QVector will just memcpy() the data on resize without calling any constructors
//template<class T>
//Q_DECLARE_TYPEINFO_BODY(SubtitleComposer::ObjectRef<T>, Q_MOVABLE_TYPE);

// QVector should probably declare this
//template<class T>
//Q_DECLARE_TYPEINFO_BODY(QVector<SubtitleComposer::ObjectRef<T>>, Q_MOVABLE_TYPE);

#endif // OBJECTREF_H
