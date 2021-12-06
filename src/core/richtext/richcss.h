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

	void parse(const QStringRef &css);

	void parseBlock(const QStringRef &selector, const QStringRef &rules);

	inline const QString & unformattedCSS() const { return m_unformatted; }

	void clear();

	QMap<QByteArray, QString> match(QSet<QString> selectors) const;

signals:
	void changed();

private:
	static inline Selector parseCssSelector(const QString &selector) { return parseCssSelector(QStringRef(&selector)); }
	static Selector parseCssSelector(const QStringRef &selector);

	static inline RuleList parseCssRules(const QString &rules) { return parseCssRules(QStringRef(&rules)); }
	static RuleList parseCssRules(const QStringRef &rules);

	void mergeCssRules(RuleList &base, const RuleList &override) const;

private:
	QString m_unformatted;
	Stylesheet m_stylesheet;
};
}

#endif // RICHCSS_H
