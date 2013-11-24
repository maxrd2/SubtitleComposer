#ifndef SERVICE_H
#define SERVICE_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtGui/QWidget>

namespace SubtitleComposer {
class ServiceBackend;

class Service : public QObject
{
	Q_OBJECT

public:
	typedef enum {
		Uninitialized = 0,
		Initialized,
	} State;

	/**
	 * @brief initialize - attempts to initialize the backend defined by prefBackendName; if that fails, attempts to initialize any other.
	 * @param widgetParent
	 * @param prefBackendName
	 * @return false if there was already an initialized backend or none could be initialized; true otherwise
	 */
	bool initialize(QWidget *widgetParent, const QString &prefBackendName = QString());

	/**
	 * @brief reinitialize - finalizes the active backend and attempts to initialize the one defined by
	 *  prefBackendName (or the active one, if is not valid a valid name); if that fails, attempts to
	 *  initialize any other.
	 * @param prefBackendName
	 * @return false if there was no initialized backend or none could be initialized; true otherwise
	 */
	bool reinitialize(const QString &prefBackendName = QString());

	/**
	 * @brief finalize - finalizes the active backend
	 */
	void finalize();

	/**
	 * @brief dummyBackendName - services should provide a dummy backend (one that implements its
	 *  operations as noops) so that the application can act reasonably even in absence of (real)
	 *  supported backends.
	 * @return
	 */
	virtual QString dummyBackendName() const = 0;

	QString activeBackendName() const;
	QStringList backendNames() const;

	inline bool isActiveBackendDummy() const;

	inline ServiceBackend * backend(const QString &name) const;
	inline ServiceBackend * activeBackend() const;

	/**
	 * @brief isApplicationClosingDown - Indicates that the application is closing down and the backends
	 *  shouldn't rely on it for some things (such as processing events).
	 * @return
	 */
	bool isApplicationClosingDown() const;

	inline int state() const;
	inline bool isInitialized() const;

public slots:
	/**
	 * @brief setApplicationClosingDown - Used to indicate the active backend that the application is closing down
	 */
	void setApplicationClosingDown();

signals:
	void backendInitialized(ServiceBackend *serviceBackend);
	void backendFinalized(ServiceBackend *serviceBackend);

protected:
	Service();
	virtual ~Service();

	/**
	 * @brief initializeBackend - attempts to initialize the backend, making it the active backend.
	 * @param backend
	 * @param widgetParent
	 * @return true if backend is the active backend after the call; false if there was already another backend initialized
	 */
	virtual bool initializeBackend(ServiceBackend *backend, QWidget *widgetParent) = 0;

	/**
	 * @brief finalizeBackend - finalizes the active backend, leaving no active backend.
	 * @param backend
	 * returns??? the previously initialized backend (or 0 if there was none)
	 */
	virtual void finalizeBackend(ServiceBackend *backend) = 0;

	void addBackend(ServiceBackend *backend);

private:
	bool initializeBackendPrivate(ServiceBackend *backend);

	QMap<QString, ServiceBackend *> m_backends;
	ServiceBackend *m_activeBackend;
	QWidget *m_widgetParent;

	bool m_applicationClosingDown;

protected:
	int m_state;

	friend class ServiceBackend;
};

int
Service::state() const
{
	return m_state;
}

bool
Service::isInitialized() const
{
	return m_state >= Service::Initialized;
}

ServiceBackend *
Service::activeBackend() const
{
	return m_activeBackend;
}

ServiceBackend *
Service::backend(const QString &backendName) const
{
	return m_backends.contains(backendName) ? m_backends[backendName] : 0;
}

bool
Service::isActiveBackendDummy() const
{
	return activeBackendName() == dummyBackendName();
}
}
#endif
