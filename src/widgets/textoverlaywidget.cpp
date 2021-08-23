/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "textoverlaywidget.h"

#include <QCoreApplication>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QResizeEvent>

using namespace SubtitleComposer;

TextOverlayWidget::TextOverlayWidget(QWidget *parent)
	: QWidget(parent)
{
	parent->installEventFilter(this);
}

TextOverlayWidget::~TextOverlayWidget()
{
}

QSize
TextOverlayWidget::minimumSizeHint() const
{
	return m_overlay.textSize();
}

bool
TextOverlayWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == parentWidget() && event->type() == QEvent::Resize)
		m_overlay.setImageSize(parentWidget()->size());

	return QWidget::eventFilter(object, event);
}

void
TextOverlayWidget::paintEvent(QPaintEvent * /*event */)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::transparent);
	const int offY = height() - contentsMargins().bottom() - m_overlay.textSize().height();
	painter.drawImage(0, offY, m_overlay.image());
}
