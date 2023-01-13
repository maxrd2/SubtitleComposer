/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOM_H
#define RICHDOM_H

#include <QString>

namespace SubtitleComposer {

class RichDocument;

class RichDOM
{
public:
	RichDOM();
	~RichDOM();

	enum NodeType {
		Root,
		Bold,
		Italic,
		Underline,
		Strikethrough,
		Font,
		Class,
		Voice,
		Invalid = -1
	};

	struct Node {
		Node(NodeType type_=Invalid, const QString &klass_=QString(), const QString &id_=QString());
		~Node();

		Node(const Node &other);
		Node & operator=(const Node &other);

		QString cssSel();

		void debugDump(QString pfx=QString());

		NodeType type;
		QString id;
		QString klass;
		quint32 nodeStart;
		quint32 nodeEnd;
		Node *next;
		Node *parent;
		Node *children;
	};

	void update(const RichDocument *doc);

private:
	friend class RichDocument;
	Node *m_root;
};

}

#endif // RICHDOM_H
