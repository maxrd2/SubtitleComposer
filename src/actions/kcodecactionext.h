/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCODECACTIONEXT_H
#define KCODECACTIONEXT_H

#include <KSelectAction>

#include <QIcon>

QT_FORWARD_DECLARE_CLASS(QTextCodec)

class KCodecActionExt : public KSelectAction
{
	Q_OBJECT

public:
	enum Mode { Open, Save };
	explicit KCodecActionExt(QObject *parent, Mode mode);
	KCodecActionExt(const QString &text, QObject *parent, Mode mode);
	KCodecActionExt(const QIcon &icon, const QString &text, QObject *parent, Mode mode);

public:
	bool setCurrentCodec(QTextCodec *codec);

Q_SIGNALS:
	void triggered(QTextCodec *codec);

protected Q_SLOTS:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	void slotActionTriggered(QAction *) override {}
#else
	void actionTriggered(QAction *) override {}
#endif

private:
	void init();

	Mode m_mode;
	QAction *m_defaultCodecAction;
	QAction *m_currentCodecAction;
	QAction *m_autodetectAction;
};

#endif
