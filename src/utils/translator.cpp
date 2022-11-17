/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "translator.h"

#include "helpers/common.h"

#include <QRegularExpression>
#include <QTextCodec>
#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QUrlQuery>
#include <QEventLoop>
#include <QDebug>

#include <KLocalizedString>

#include <KIO/Job>

using namespace SubtitleComposer;

#define MULTIPART_DATA_BOUNDARY "----------nOtA5FcjrNZuZ3TMioysxHGGCO69vA5iYysdBTL2osuNwOjcCfU7uiN"

Translator::Translator(QObject *parent) :
	QObject(parent),
	m_currentTransferJob(0),
	m_inputLanguage(Language::INVALID),
	m_outputLanguage(Language::INVALID),
	m_lastReceivedChunk(0)
{
	connect(this, QOverload<const QString &>::of(&Translator::finished), this, QOverload<>::of(&Translator::finished));
	connect(this, QOverload<const QString &>::of(&Translator::finishedWithError), this, QOverload<>::of(&Translator::finished));
}

Translator::~Translator()
{}

QString
Translator::inputText() const
{
	return m_inputTextChunks.join(QString());
}

QString
Translator::outputText() const
{
	return m_outputText;
}

Language::Value
Translator::inputLanguage() const
{
	return m_inputLanguage;
}

Language::Value
Translator::outputLanguage() const
{
	return m_outputLanguage;
}

int
Translator::chunksCount() const
{
	return m_inputTextChunks.count();
}

bool
Translator::isFinished() const
{
	return isFinishedWithError() || m_lastReceivedChunk == chunksCount();
}

bool
Translator::isFinishedWithError() const
{
	return !m_errorMessage.isEmpty();
}

bool
Translator::isAborted() const
{
	return m_aborted;
}

QString
Translator::errorMessage() const
{
	return m_errorMessage;
}

bool
Translator::syncTranslate(const QString &text, Language::Value inputLang, Language::Value outputLang, ProgressDialog *progressDialog)
{
	if(progressDialog) {
		connect(this, &Translator::progress, progressDialog, &ProgressDialog::setValue);
		connect(progressDialog, &ProgressDialog::rejected, this, &Translator::abort);

		progressDialog->setMinimum(0);
		progressDialog->setMaximum(100);
		progressDialog->show();
	}

	QEventLoop loop;
	connect(this, static_cast<void(Translator::*)(void)>(&Translator::finished), &loop, &QEventLoop::quit);
	translate(text, inputLang, outputLang);
	loop.exec();

	if(progressDialog)
		progressDialog->hide();

	return !isFinishedWithError();
}

static int
findOptimalSplitIndex(const QString &text, int fromIndex = 0)
{
	int lastTargetIndex = qMin(fromIndex + Translator::MaxChunkSize, text.size() - 1);

	int index = text.lastIndexOf('\n', lastTargetIndex);
	if(index >= (fromIndex + Translator::MaxChunkSize * (3 / 4)))
		return index;

	index = text.lastIndexOf(' ', lastTargetIndex);
	if(index >= (fromIndex + Translator::MaxChunkSize * (3 / 4)))
		return index;

	// text it's really weird (probably garbage)... we just split it anywhere
	return fromIndex + Translator::MaxChunkSize;
}

void
Translator::translate(const QString &text, Language::Value inputLanguage, Language::Value outputLanguage)
{
	m_inputTextChunks.clear();

	for(int index = 0, splitIndex; index < text.length(); index = splitIndex + 1) {
		splitIndex = findOptimalSplitIndex(text, index);
		m_inputTextChunks << text.mid(index, splitIndex - index + 1);
	}
	m_lastReceivedChunk = 0;

	Q_ASSERT(text == inputText());

	m_outputText.clear();
	m_inputLanguage = inputLanguage;
	m_outputLanguage = outputLanguage;
	m_errorMessage.clear();
	m_aborted = false;

	startChunkDownload(1);
}

void
Translator::abort()
{
	if(m_currentTransferJob) {
		m_currentTransferJob->kill();   // deletes the job
		m_aborted = true;
		m_errorMessage = i18n("Operation canceled by user");
		emit finishedWithError(m_errorMessage);
	}
}

// "Content-type" => "application/x-www-form-urlencoded"
QByteArray
Translator::prepareUrlEncodedData(const QMap<QString, QString> &params)
{
	QUrlQuery query;
	for(QMap<QString, QString>::ConstIterator it = params.begin(), end = params.end(); it != end; ++it)
		query.addQueryItem(it.key(), it.value());
	return query.toString().toUtf8();
}

// "Content-type" => "multipart/form-data; boundary=" MULTIPART_DATA_BOUNDARY
QByteArray
Translator::prepareMultipartData(const QMap<QString, QString> &params)
{
	QByteArray data;

	for(QMap<QString, QString>::ConstIterator it = params.begin(), end = params.end(); it != end; ++it) {
		data.append("--");
		data.append(MULTIPART_DATA_BOUNDARY);
		data.append("\r\n");
		data.append("Content-Disposition: form-data; name=\"");
		data.append(it.key().toUtf8());
		data.append("\"\r\n\r\n");
		data.append(it.value().toUtf8());
		data.append("\r\n");
	}

	data.append("--");
	data.append(MULTIPART_DATA_BOUNDARY);
	data.append("--");

	return data;
}

void
Translator::startChunkDownload(int chunkNumber)
{
	QMap<QString, QString> params;
	params["prev"] = "_t";
	params["sl"] = Language::code(m_inputLanguage);
	params["tl"] = Language::code(m_outputLanguage);
	params["text"] = m_inputTextChunks.at(chunkNumber - 1);

	// QByteArray postData = prepareMultipartData( params );
	QByteArray postData = prepareUrlEncodedData(params);

	m_currentTransferJob = KIO::http_post(QUrl("https://translate.google.com/translate_t"), postData, KIO::HideProgressInfo);

	m_currentTransferJob->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
	m_currentTransferJob->setTotalSize(postData.length());

	// FIXME: KIO::TransferJob::percent is private signal... amd translator doesn't work anyways
//	connect(m_currentTransferJob, QOverload<KJob *, unsigned long>::of(&KIO::TransferJob::percent), this, &Translator::onTransferJobProgress);
	connect(m_currentTransferJob, &KJob::result, this, &Translator::onTransferJobResult);
	connect(m_currentTransferJob, &KIO::TransferJob::data, this, &Translator::onTransferJobData);

	m_currentTransferData.clear();

	m_currentTransferJob->start();
}

void
Translator::onTransferJobProgress(KJob * /*job */, unsigned long percent)
{
	const double r = 1.0 / chunksCount();
	int percentage = (int)(m_lastReceivedChunk * 100.0 * r + percent * r);
	emit progress(percentage);
}

void
Translator::onTransferJobData(KIO::Job * /*job */, const QByteArray &data)
{
	m_currentTransferData.append(data);
}

void
Translator::onTransferJobResult(KJob *job)
{
	m_currentTransferJob = 0;

	if(job->error()) {
		m_aborted = false;
		m_errorMessage = job->errorString();
		qDebug() << m_errorMessage;
		emit finishedWithError(m_errorMessage);
		return;
	}

	QTextCodec *codec = QTextCodec::codecForHtml(m_currentTransferData, QTextCodec::codecForName("UTF-8"));
	QString content = codec->toUnicode(m_currentTransferData);

	staticRE$(resultStartRegExp, "^.*<textarea name=utrans [^>]+>", REi);
	if(content.contains(resultStartRegExp)) {
		content.remove(resultStartRegExp);
	} else {
		m_errorMessage = i18n("Unexpected contents received from translation service");
		emit finishedWithError(m_errorMessage);
		return;
	}

	staticRE$(resultEndRegExp, "</textarea>.*$", REi);
	if(content.contains(resultEndRegExp)) {
		content.remove(resultEndRegExp);
	} else {
		m_errorMessage = i18n("Unexpected contents received from translation service");
		emit finishedWithError(m_errorMessage);
		return;
	}

	replaceHTMLEntities(content);
	content.replace(QLatin1String("&quot;"), QLatin1String("\""));
	content.replace(QLatin1String("&lt;"), QLatin1String("<"));
	content.replace(QLatin1String("&gt;"), QLatin1String(">"));

	staticRE$(reHtmlTag, "< *(/?) *([a-z]+) *(/?) *>", REi);
	content.replace(reHtmlTag, $("<\\1\\2\\3>"));
	staticRE$(reHtmlBreak, " ?<br ?/?> ?", REi);
	content.replace(reHtmlBreak, $("\n"));
	staticRE$(reLineBreak, "\\n{2,}", REi);
	content.replace(reLineBreak, $("\n"));

	if(!m_outputText.isEmpty())
		m_outputText += "\n";
	m_outputText += content;

	m_lastReceivedChunk += 1;

	if(m_lastReceivedChunk >= chunksCount())
		emit finished(m_outputText);
	else
		startChunkDownload(m_lastReceivedChunk + 1);
}

QString &
Translator::replaceHTMLEntities(QString &text)
{
	staticRE$(namedEntitiesRegExp, "&([a-zA-Z]+);");
	const QMap<QString, QChar> &namedEntities = Translator::namedEntities();
	QRegularExpressionMatchIterator it = namedEntitiesRegExp.globalMatch(text);
	while(it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QString entityName = match.captured(1).toLower();
		if(namedEntities.contains(entityName)) {
			text.replace(match.capturedStart(), match.capturedLength(), namedEntities[entityName]);
		}
	}

	staticRE$(unnamedB10EntitiesRegExp, "&#(\\d{2,4});");
	it = unnamedB10EntitiesRegExp.globalMatch(text);
	while(it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QChar entityValue = QChar(match.captured(1).toUInt(0, 10));
		text.replace(match.capturedStart(), match.capturedLength(), entityValue);
	}

	staticRE$(unnamedB16EntitiesRegExp, "&#x([\\da-fA-F]{2,4});");
	it = unnamedB16EntitiesRegExp.globalMatch(text);
	while(it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QChar entityValue = QChar(match.captured(1).toUInt(0, 16));
		text.replace(match.capturedStart(), match.capturedLength(), entityValue);
	}

	return text;
}

const QMap<QString, QChar> &
Translator::namedEntities()
{
	static QMap<QString, QChar> entities;
	if(entities.empty()) {
		entities["quot"] = QChar(34);
		entities["amp"] = QChar(38);
		entities["lt"] = QChar(60);
		entities["gt"] = QChar(62);
		entities["oelig"] = QChar(338);
		entities["oelig"] = QChar(339);
		entities["scaron"] = QChar(352);
		entities["scaron"] = QChar(353);
		entities["yuml"] = QChar(376);
		entities["circ"] = QChar(710);
		entities["tilde"] = QChar(732);
		entities["ensp"] = QChar(8194);
		entities["emsp"] = QChar(8195);
		entities["thinsp"] = QChar(8201);
		entities["zwnj"] = QChar(8204);
		entities["zwj"] = QChar(8205);
		entities["lrm"] = QChar(8206);
		entities["rlm"] = QChar(8207);
		entities["ndash"] = QChar(8211);
		entities["mdash"] = QChar(8212);
		entities["lsquo"] = QChar(8216);
		entities["rsquo"] = QChar(8217);
		entities["sbquo"] = QChar(8218);
		entities["ldquo"] = QChar(8220);
		entities["rdquo"] = QChar(8221);
		entities["bdquo"] = QChar(8222);
		entities["dagger"] = QChar(8224);
		entities["dagger"] = QChar(8225);
		entities["permil"] = QChar(8240);
		entities["lsaquo"] = QChar(8249);
		entities["rsaquo"] = QChar(8250);
		entities["euro"] = QChar(8364);
		entities["fnof"] = QChar(402);
		entities["alpha"] = QChar(913);
		entities["beta"] = QChar(914);
		entities["gamma"] = QChar(915);
		entities["delta"] = QChar(916);
		entities["epsilon"] = QChar(917);
		entities["zeta"] = QChar(918);
		entities["eta"] = QChar(919);
		entities["theta"] = QChar(920);
		entities["iota"] = QChar(921);
		entities["kappa"] = QChar(922);
		entities["lambda"] = QChar(923);
		entities["mu"] = QChar(924);
		entities["nu"] = QChar(925);
		entities["xi"] = QChar(926);
		entities["omicron"] = QChar(927);
		entities["pi"] = QChar(928);
		entities["rho"] = QChar(929);
		entities["sigma"] = QChar(931);
		entities["tau"] = QChar(932);
		entities["upsilon"] = QChar(933);
		entities["phi"] = QChar(934);
		entities["chi"] = QChar(935);
		entities["psi"] = QChar(936);
		entities["omega"] = QChar(937);
		entities["alpha"] = QChar(945);
		entities["beta"] = QChar(946);
		entities["gamma"] = QChar(947);
		entities["delta"] = QChar(948);
		entities["epsilon"] = QChar(949);
		entities["zeta"] = QChar(950);
		entities["eta"] = QChar(951);
		entities["theta"] = QChar(952);
		entities["iota"] = QChar(953);
		entities["kappa"] = QChar(954);
		entities["lambda"] = QChar(955);
		entities["mu"] = QChar(956);
		entities["nu"] = QChar(957);
		entities["xi"] = QChar(958);
		entities["omicron"] = QChar(959);
		entities["pi"] = QChar(960);
		entities["rho"] = QChar(961);
		entities["sigmaf"] = QChar(962);
		entities["sigma"] = QChar(963);
		entities["tau"] = QChar(964);
		entities["upsilon"] = QChar(965);
		entities["phi"] = QChar(966);
		entities["chi"] = QChar(967);
		entities["psi"] = QChar(968);
		entities["omega"] = QChar(969);
		entities["thetasym"] = QChar(977);
		entities["upsih"] = QChar(978);
		entities["piv"] = QChar(982);
		entities["bull"] = QChar(8226);
		entities["hellip"] = QChar(8230);
		entities["prime"] = QChar(8242);
		entities["prime"] = QChar(8243);
		entities["oline"] = QChar(8254);
		entities["frasl"] = QChar(8260);
		entities["weierp"] = QChar(8472);
		entities["image"] = QChar(8465);
		entities["real"] = QChar(8476);
		entities["trade"] = QChar(8482);
		entities["alefsym"] = QChar(8501);
		entities["larr"] = QChar(8592);
		entities["uarr"] = QChar(8593);
		entities["rarr"] = QChar(8594);
		entities["darr"] = QChar(8595);
		entities["harr"] = QChar(8596);
		entities["crarr"] = QChar(8629);
		entities["larr"] = QChar(8656);
		entities["uarr"] = QChar(8657);
		entities["rarr"] = QChar(8658);
		entities["darr"] = QChar(8659);
		entities["harr"] = QChar(8660);
		entities["forall"] = QChar(8704);
		entities["part"] = QChar(8706);
		entities["exist"] = QChar(8707);
		entities["empty"] = QChar(8709);
		entities["nabla"] = QChar(8711);
		entities["isin"] = QChar(8712);
		entities["notin"] = QChar(8713);
		entities["ni"] = QChar(8715);
		entities["prod"] = QChar(8719);
		entities["sum"] = QChar(8721);
		entities["minus"] = QChar(8722);
		entities["lowast"] = QChar(8727);
		entities["radic"] = QChar(8730);
		entities["prop"] = QChar(8733);
		entities["infin"] = QChar(8734);
		entities["ang"] = QChar(8736);
		entities["and"] = QChar(8743);
		entities["or"] = QChar(8744);
		entities["cap"] = QChar(8745);
		entities["cup"] = QChar(8746);
		entities["int"] = QChar(8747);
		entities["there4"] = QChar(8756);
		entities["sim"] = QChar(8764);
		entities["cong"] = QChar(8773);
		entities["asymp"] = QChar(8776);
		entities["ne"] = QChar(8800);
		entities["equiv"] = QChar(8801);
		entities["le"] = QChar(8804);
		entities["ge"] = QChar(8805);
		entities["sub"] = QChar(8834);
		entities["sup"] = QChar(8835);
		entities["nsub"] = QChar(8836);
		entities["sube"] = QChar(8838);
		entities["supe"] = QChar(8839);
		entities["oplus"] = QChar(8853);
		entities["otimes"] = QChar(8855);
		entities["perp"] = QChar(8869);
		entities["sdot"] = QChar(8901);
		entities["lceil"] = QChar(8968);
		entities["rceil"] = QChar(8969);
		entities["lfloor"] = QChar(8970);
		entities["rfloor"] = QChar(8971);
		entities["lang"] = QChar(9001);
		entities["rang"] = QChar(9002);
		entities["loz"] = QChar(9674);
		entities["spades"] = QChar(9824);
		entities["clubs"] = QChar(9827);
		entities["hearts"] = QChar(9829);
		entities["diams"] = QChar(9830);
		entities["nbsp"] = QChar(160);
		entities["iexcl"] = QChar(161);
		entities["cent"] = QChar(162);
		entities["pound"] = QChar(163);
		entities["curren"] = QChar(164);
		entities["yen"] = QChar(165);
		entities["brvbar"] = QChar(166);
		entities["sect"] = QChar(167);
		entities["uml"] = QChar(168);
		entities["copy"] = QChar(169);
		entities["ordf"] = QChar(170);
		entities["laquo"] = QChar(171);
		entities["not"] = QChar(172);
		entities["shy"] = QChar(173);
		entities["reg"] = QChar(174);
		entities["macr"] = QChar(175);
		entities["deg"] = QChar(176);
		entities["plusmn"] = QChar(177);
		entities["sup2"] = QChar(178);
		entities["sup3"] = QChar(179);
		entities["acute"] = QChar(180);
		entities["micro"] = QChar(181);
		entities["para"] = QChar(182);
		entities["middot"] = QChar(183);
		entities["cedil"] = QChar(184);
		entities["sup1"] = QChar(185);
		entities["ordm"] = QChar(186);
		entities["raquo"] = QChar(187);
		entities["frac14"] = QChar(188);
		entities["frac12"] = QChar(189);
		entities["frac34"] = QChar(190);
		entities["iquest"] = QChar(191);
		entities["agrave"] = QChar(192);
		entities["aacute"] = QChar(193);
		entities["acirc"] = QChar(194);
		entities["atilde"] = QChar(195);
		entities["auml"] = QChar(196);
		entities["aring"] = QChar(197);
		entities["aelig"] = QChar(198);
		entities["ccedil"] = QChar(199);
		entities["egrave"] = QChar(200);
		entities["eacute"] = QChar(201);
		entities["ecirc"] = QChar(202);
		entities["euml"] = QChar(203);
		entities["igrave"] = QChar(204);
		entities["iacute"] = QChar(205);
		entities["icirc"] = QChar(206);
		entities["iuml"] = QChar(207);
		entities["eth"] = QChar(208);
		entities["ntilde"] = QChar(209);
		entities["ograve"] = QChar(210);
		entities["oacute"] = QChar(211);
		entities["ocirc"] = QChar(212);
		entities["otilde"] = QChar(213);
		entities["ouml"] = QChar(214);
		entities["times"] = QChar(215);
		entities["oslash"] = QChar(216);
		entities["ugrave"] = QChar(217);
		entities["uacute"] = QChar(218);
		entities["ucirc"] = QChar(219);
		entities["uuml"] = QChar(220);
		entities["yacute"] = QChar(221);
		entities["thorn"] = QChar(222);
		entities["szlig"] = QChar(223);
		entities["agrave"] = QChar(224);
		entities["aacute"] = QChar(225);
		entities["acirc"] = QChar(226);
		entities["atilde"] = QChar(227);
		entities["auml"] = QChar(228);
		entities["aring"] = QChar(229);
		entities["aelig"] = QChar(230);
		entities["ccedil"] = QChar(231);
		entities["egrave"] = QChar(232);
		entities["eacute"] = QChar(233);
		entities["ecirc"] = QChar(234);
		entities["euml"] = QChar(235);
		entities["igrave"] = QChar(236);
		entities["iacute"] = QChar(237);
		entities["icirc"] = QChar(238);
		entities["iuml"] = QChar(239);
		entities["eth"] = QChar(240);
		entities["ntilde"] = QChar(241);
		entities["ograve"] = QChar(242);
		entities["oacute"] = QChar(243);
		entities["ocirc"] = QChar(244);
		entities["otilde"] = QChar(245);
		entities["ouml"] = QChar(246);
		entities["divide"] = QChar(247);
		entities["oslash"] = QChar(248);
		entities["ugrave"] = QChar(249);
		entities["uacute"] = QChar(250);
		entities["ucirc"] = QChar(251);
		entities["uuml"] = QChar(252);
		entities["yacute"] = QChar(253);
		entities["thorn"] = QChar(254);
		entities["yuml"] = QChar(255);
	}

	return entities;
}


