/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "translateengine.h"

#include <QNetworkReply>
#include <set>

using namespace SubtitleComposer;

struct TranslateEngine::ProgressHelper {
	std::set<QNetworkReply *> list;
	int active;
	int total;
};

TranslateEngine::ProgressLock::ProgressLock(TranslateEngine *e, const QString &progressText)
	: te(e)
{
	if(!te->m_progress)
		te->m_progress = new QProgressDialog(progressText, QString(), 0, 1, qobject_cast<QWidget *>(te->parent()));
	te->m_progress->setModal(true);
	te->m_progress->show();
	te->translateStart();
}

TranslateEngine::ProgressLock::~ProgressLock()
{
	te->translateDone();
}

TranslateEngine::TranslateEngine(QObject *parent)
	: QObject(parent)
	, m_progress(nullptr)
	, m_ph(nullptr)
{
}

void
TranslateEngine::translateDone()
{
	m_ph->active--;

	Q_ASSERT(m_progress);
	m_progress->setValue(m_ph->total - m_ph->active);

	if(m_ph->active == 0) {
		delete m_ph;
		m_ph = nullptr;

		m_progress->deleteLater();
		m_progress = nullptr;
		emit translated();
	}
};

void
TranslateEngine::translateStart()
{
	if(!m_ph) {
		m_ph = new ProgressHelper();

		Q_ASSERT(m_progress);
		m_progress->setCancelButtonText(i18n("Abort"));
		connect(m_progress, &QProgressDialog::canceled, this, [&](){
			for(auto it = m_ph->list.cbegin(); it != m_ph->list.cend(); ++it) {
				disconnect(*it, &QNetworkReply::finished, nullptr, nullptr);
				(*it)->abort();
				(*it)->deleteLater();
				m_ph->list.erase(it);
			}
			delete m_ph;
			m_ph = nullptr;
			m_progress->deleteLater();
			m_progress = nullptr;
		});
	}

	m_ph->active++;
	m_ph->total++;
}

void
TranslateEngine::sendRequest(QNetworkAccessManager *nm, const QNetworkRequest &request, const QByteArray &data, std::function<void(QNetworkReply*)> callback)
{
	translateStart();

	Q_ASSERT(m_progress);
	m_progress->setMaximum(m_ph->total);
	m_progress->setValue(m_ph->total - m_ph->active);

	QNetworkReply *res = data.isNull() ? nm->get(request) : nm->post(request, data);
	m_ph->list.insert(res);
	connect(res, &QNetworkReply::finished, this, [=](){
		m_ph->list.erase(res);
		res->deleteLater();
		callback(res);
		translateDone();
	});
}
