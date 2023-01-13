/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdom.h"

#include "core/richtext/richdocument.h"
#include "helpers/common.h"

#include <stack>

#include <QStringBuilder>
#include <QTextFormat>


using namespace SubtitleComposer;


RichDOM::RichDOM()
	: m_root(new Node())
{
}

RichDOM::~RichDOM()
{
	delete m_root;
}


RichDOM::Node::Node(NodeType type_, const QString &klass_, const QString &id_)
	: type(type_),
	  id(id_),
	  klass(klass_),
	  next(nullptr),
	  parent(nullptr),
	  children(nullptr)
{
}

RichDOM::Node::~Node()
{
	delete next;
	delete children;
}

RichDOM::Node::Node(const Node &o)
	: type(o.type),
	  id(o.id),
	  klass(o.klass),
	  nodeStart(o.nodeStart),
	  nodeEnd(o.nodeEnd),
	  next(nullptr),
	  parent(nullptr),
	  children(nullptr)
{
}

RichDOM::Node &
RichDOM::Node::operator=(const Node &o)
{
	type = o.type;
	id = o.id;
	klass = o.klass;
	nodeStart = o.nodeStart;
	nodeEnd = o.nodeEnd;
	return *this;
}

QString
RichDOM::Node::cssSel()
{
	static const QString names[] = {
		$("body"), // Root
		$("b"),    // Bold
		$("i"),    // Italic
		$("u"),    // Underline
		$("s"),    // Strikethrough
		$("font"), // Font
		$("c"),    // Class
		$("v"),    // Voice
	};

	QString nn = names[type];
	if(!klass.isEmpty())
		nn += QChar('.') + klass;
	if(!id.isEmpty())
		nn += QChar('#') + id;
	return nn;
}

static RichDOM::Node *
nodeOpen(RichDOM::Node *parent, quint32 pos, RichDOM::NodeType type, const QString &klass=QString(), const QString &id=QString())
{
	auto *node = new RichDOM::Node(type, klass, id);
	node->nodeStart = pos;

	auto *p = node->parent = parent;
	if(!p->children) {
		p->children = node;
	} else {
		p = p->children;
		while(p->next)
			p = p->next;
		p->next = node;
	}

	return node;
}

static RichDOM::Node *
nodeClose(RichDOM::Node *last, int pos, RichDOM::NodeType type, const QString &klass=QString(), const QString &id=QString())
{
	// find node to close up in the tree
	std::stack<RichDOM::Node *> tmp;
	while(last->parent) {
		// close child/wanted nodes
		last->nodeEnd = pos;

		if(last->type == type && last->klass == klass && last->id == id) {
			// store parent of wanted node
			last = last->parent;
			break;
		}

		// store closed child nodes
		tmp.push(last);
		last = last->parent;
	}

	// add clones of closed child nodes
	while(!tmp.empty()) {
		auto *n = tmp.top();
		tmp.pop();
		last = nodeOpen(last, pos, n->type, n->klass, n->id);
	}

	return last;
}

void
RichDOM::update(const RichDocument *doc)
{
	bool fB = false;
	bool fI = false;
	bool fU = false;
	bool fS = false;
	QRgb fC = 0;
	QSet<QString> fClass;
	QString fVoice; // <v:speaker name> - can't be nested... right? No need for QSet<QString>

	delete m_root;
	m_root = new Node(Root);
	m_root->nodeStart = 0;

	Node *last = m_root;

	QTextBlock bi = doc->begin();
	for(;;) {
		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			const QTextFragment &f = it.fragment();
			if(!f.isValid())
				continue;
			const QTextCharFormat &format = f.charFormat();
			const QSet<QString> &cl = format.property(RichDocument::Class).value<QSet<QString>>();
			for(auto it = fClass.begin(); it != fClass.end();) {
				if(cl.contains(*it)) {
					++it;
					continue;
				}
				last = nodeClose(last, f.position(), Class, *it);
				it = fClass.erase(it);
			}
			for(auto it = cl.cbegin(); it != cl.cend(); ++it) {
				if(fClass.contains(*it))
					continue;
				last = nodeOpen(last, f.position(), Class, *it);
				fClass.insert(*it);
			}
			const QString &vt = format.property(RichDocument::Voice).value<QString>();
			if(fVoice != vt) {
				if(!fVoice.isEmpty())
					last = nodeClose(last, f.position(), Voice, fVoice);
				fVoice = vt;
				last = nodeOpen(last, f.position(), Voice, fVoice);
			}
			if(fB != (format.fontWeight() == QFont::Bold)) {
				if((fB = !fB))
					last = nodeOpen(last, f.position(), Bold);
				else
					last = nodeClose(last, f.position(), Bold);
			}
			if(fI != format.fontItalic()) {
				if((fI = !fI))
					last = nodeOpen(last, f.position(), Italic);
				else
					last = nodeClose(last, f.position(), Italic);
			}
			if(fU != format.fontUnderline()) {
				if((fU = !fU))
					last = nodeOpen(last, f.position(), Underline);
				else
					last = nodeClose(last, f.position(), Underline);
			}
			if(fS != format.fontStrikeOut()) {
				if((fS = !fS))
					last = nodeOpen(last, f.position(), Strikethrough);
				else
					last = nodeClose(last, f.position(), Strikethrough);
			}
			const QRgb fg = format.foreground().style() != Qt::NoBrush ? format.foreground().color().toRgb().rgb() : 0;
			if(fC != fg) {
				if(fC)
					last = nodeClose(last, f.position(), Font);
				if((fC = fg))
					last = nodeOpen(last, f.position(), Font);
			}
		}
		bi = bi.next();
		if(bi == doc->end()) {
			const int pos = doc->length();
			while(last) {
				// close remaining node
				last->nodeEnd = pos;
				last = last->parent;
			}
			return;
		}
	}
	// unreachable
}

void
RichDOM::Node::debugDump(QString pfx)
{
	QString f = pfx % $(" > ") % cssSel();
	qDebug() << f;
	if(children)
		children->debugDump(f);
	if(next)
		next->debugDump(pfx);
}
