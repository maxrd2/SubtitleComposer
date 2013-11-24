#ifndef KCODECACTIONEXT_H
#define KCODECACTIONEXT_H

/***************************************************************************
 *   Copyright (C) 2007-2010 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
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

#include <kcodecaction.h>

class KCodecActionExt : public KCodecAction
{
	Q_OBJECT

public:
	explicit KCodecActionExt(QObject *parent, bool showAutoOptions = false, bool showDefault = false);
	KCodecActionExt(const QString &text, QObject *parent, bool showAutoOptions = false, bool showDefault = false);
	KCodecActionExt(const KIcon &icon, const QString &text, QObject *parent, bool showAutoOptions = false, bool showDefault = false);

public:
	KEncodingDetector::AutoDetectScript currentAutoDetectScript() const;
	bool setCurrentAutoDetectScript(KEncodingDetector::AutoDetectScript);

protected Q_SLOTS:
	virtual void actionTriggered(QAction *);

private:
	void init(bool showDefault);

	bool m_showDefault;
	KAction *m_autodetectAction;
};

#endif
