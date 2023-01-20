/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHCSS_H
#define RICHCSS_H

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <list>
#include <memory>

namespace SubtitleComposer {

class RichCSS : public QObject
{
	Q_OBJECT

public:
	RichCSS(QObject *parent=nullptr);
	RichCSS(RichCSS &other);

	RichCSS & operator=(const RichCSS &rhs);

	typedef QString Selector;
	struct Rule { QByteArray name; QString value; };
	class RuleList : public QVector<Rule> {
	public:
		QStringList toStringList();
		inline QString toString() { return toStringList().join(QString()); }
	};
	struct Block { Selector selector; RuleList rules; };
	typedef QVector<Block> Stylesheet;

	inline void parse(const QString &css) { parse(css.data()); }
	void parse(const QChar *css);

	inline const QString & unformattedCSS() const { return m_unformatted; }

	void clear();

	QMap<QByteArray, QString> match(QSet<QString> selectors) const;

	/**
	 * @return list of all defined classes
	 */
	QSet<QString> classes() const;

signals:
	void changed();

private:
	static Selector parseCssSelector(const QChar **stylesheet);
	static RuleList parseCssRules(const QChar **stylesheet);
	static QString parseCssKey(const QChar **stylesheet);
	static QString parseCssValue(const QChar **stylesheet);

	void mergeCssRules(RuleList &base, const RuleList &override) const;

private:
	QString m_unformatted;
	Stylesheet m_stylesheet;
};
}

#endif // RICHCSS_H
