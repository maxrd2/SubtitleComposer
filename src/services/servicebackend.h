#ifndef SERVICEBACKEND_H
#define SERVICEBACKEND_H

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

#include "service.h"
#include "../config/appconfiggroup.h"
#include "../config/appconfiggroupwidget.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QWidget>

namespace SubtitleComposer {
class ServiceBackend : public QObject
{
	Q_OBJECT

	friend class Service;
	friend class Player;
	friend class Decoder;

public:
/// ownership of the config object is transferred to this object
	ServiceBackend(Service *service, const QString &name, AppConfigGroup *config);
	virtual ~ServiceBackend();

	inline const QString & name() const { return m_name; }

	inline const AppConfigGroup * config() const { return m_config; }

// If possible (i.e., configs are compatible), copies the config object into
// the service backend config. Ownership of config object it's not transferred.
	void setConfig(const AppConfigGroup *const config);
	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget *parent) = 0;

	bool isDummy() const;

protected:
	/**
	 * @brief isInitialized - There can only be one initialized backend at the time (the active
	 *  backend). Since the active backend is also guaranteed to be initialized, this
	 *  return the same as isActiveBackend() method.
	 * @return true if initialize() has been successfully on this backend; false otherwise
	 */
	bool isInitialized() const;
	bool isActiveBackend() const;

	/**
	 * @brief initialize - Perform any required initialization
	 * @param widgetParent
	 * @return
	 */
	virtual QWidget * initialize(QWidget *widgetParent) = 0;

	/**
	 * @brief finalize - Cleanup anything that has been initialized by initialize(), excluding the
	 *  videoWidget() which is destroyed after calling fninalize() (all references to it must be
	 *  cleaned up, however)
	 */
	virtual void finalize() = 0;

	inline Service * service() const { return m_service; }

private:
	Service *m_service;
	QString m_name;
	AppConfigGroup *m_config;
};
}
#endif
