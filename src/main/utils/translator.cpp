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

#include "translator.h"
#include "../../common/qxtsignalwaiter.h"

#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QBoxLayout>

#include <KDebug>
#include <KLocale>
#include <KDialog>

#include <kjob.h>
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
	connect(this, SIGNAL(finished(const QString &)), this, SIGNAL(finished()));
	connect(this, SIGNAL(finishedWithError(const QString &)), this, SIGNAL(finished()));
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
		connect(this, SIGNAL(progress(int)), progressDialog, SLOT(setValue(int)));
		connect(progressDialog, SIGNAL(cancelClicked()), this, SLOT(abort()));

		progressDialog->setMinimum(0);
		progressDialog->setMaximum(100);
		progressDialog->show();
	}

	QxtSignalWaiter finishedSignalWaiter(this, SIGNAL(finished()));
	finishedSignalWaiter.setProcessEventFlags(QEventLoop::AllEvents);
	translate(text, inputLang, outputLang);
	finishedSignalWaiter.wait();

	if(progressDialog)
		progressDialog->hide();

	return !isFinishedWithError();
}

static int
findOptimalSplitIndex(const QString &text, int fromIndex = 0)
{
	int lastTargetIndex = qMin(fromIndex + Translator::MaxChunkSize, text.count() - 1);

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
		m_errorMessage = i18n("Operation cancelled by user");
		emit finishedWithError(m_errorMessage);
	}
}

// "Content-type" => "application/x-www-form-urlencoded"
QByteArray
Translator::prepareUrlEncodedData(const QMap<QString, QString> &params)
{
	QByteArray data;

	QUrl url;
	for(QMap<QString, QString>::ConstIterator it = params.begin(), end = params.end(); it != end; ++it)
		url.addQueryItem(it.key(), it.value());

	return url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority | QUrl::RemovePath).remove(0, 1);
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

	m_currentTransferJob = KIO::http_post(KUrl("http://translate.google.com/translate_t"), postData, KIO::HideProgressInfo);

//  m_currentTransferJob->addMetaData( "content-type", "Content-Type: multipart/form-data; boundary=" MULTIPART_DATA_BOUNDARY );
	m_currentTransferJob->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
	m_currentTransferJob->setTotalSize(postData.length());

	connect(m_currentTransferJob, SIGNAL(percent(KJob *, unsigned long)), this, SLOT(onTransferJobProgress(KJob *, unsigned long)));
	connect(m_currentTransferJob, SIGNAL(result(KJob *)), this, SLOT(onTransferJobResult(KJob *)));
	connect(m_currentTransferJob, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(onTransferJobData(KIO::Job *, const QByteArray &)));

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
		kDebug() << m_errorMessage;
		emit finishedWithError(m_errorMessage);
		return;
	}

	QTextCodec *codec = QTextCodec::codecForHtml(m_currentTransferData, QTextCodec::codecForName("UTF-8"));
	QString content = codec->toUnicode(m_currentTransferData);

	QRegExp resultStartRegExp("^.*<textarea name=utrans [^>]+>", Qt::CaseInsensitive);
	if(content.contains(resultStartRegExp)) {
		content.remove(resultStartRegExp);
	} else {
		m_errorMessage = i18n("Unexpected contents received from translation service");
		emit finishedWithError(m_errorMessage);
		return;
	}

	QRegExp resultEndRegExp("</textarea>.*$", Qt::CaseInsensitive);
	if(content.contains(resultEndRegExp)) {
		content.remove(resultEndRegExp);
	} else {
		m_errorMessage = i18n("Unexpected contents received from translation service");
		emit finishedWithError(m_errorMessage);
		return;
	}

	replaceHTMLEntities(content);
	content.replace("&quot;", "\"");
	content.replace("&lt;", "<");
	content.replace("&gt;", ">");

	content.replace(QRegExp("< *(/?) *([a-z]+) *(/?) *>", Qt::CaseInsensitive), "<\\1\\2\\3>");
	content.replace(QRegExp(" ?<br ?/?> ?", Qt::CaseInsensitive), "\n");
	content.replace(QRegExp("\n{2,}", Qt::CaseInsensitive), "\n");

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
	const QMap<QString, QChar> &namedEntities = Translator::namedEntities();
	static QRegExp namedEntitiesRegExp("&([a-zA-Z]+);");
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = namedEntitiesRegExp.indexIn(text, offsetIndex)) != -1;) {
		QString entityName = namedEntitiesRegExp.cap(1).toLower();
		if(namedEntities.contains(entityName)) {
			text.replace(matchedIndex, namedEntitiesRegExp.matchedLength(), namedEntities[entityName]);
			offsetIndex = matchedIndex + 1;
		} else {
			offsetIndex = matchedIndex + namedEntitiesRegExp.matchedLength();
		}
	}

	static QRegExp unnamedB10EntitiesRegExp("&#(\\d{2,4});");
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = unnamedB10EntitiesRegExp.indexIn(text, offsetIndex)) != -1; offsetIndex = matchedIndex + 1) {
		QChar entityValue(unnamedB10EntitiesRegExp.cap(1).toUInt(0, 10));
		text.replace(matchedIndex, unnamedB10EntitiesRegExp.matchedLength(), entityValue);
	}

	static QRegExp unnamedB16EntitiesRegExp("&#x([\\da-fA-F]{2,4});");
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = unnamedB16EntitiesRegExp.indexIn(text, offsetIndex)) != -1; offsetIndex = matchedIndex + 1) {
		QChar entityValue(unnamedB16EntitiesRegExp.cap(1).toUInt(0, 16));
		text.replace(matchedIndex, unnamedB16EntitiesRegExp.matchedLength(), entityValue);
	}

	return text;
}

const QMap<QString, QChar> &
Translator::namedEntities()
{
	static QMap<QString, QChar> entities;
	if(entities.empty()) {
		entities["quot"] = 34;
		entities["amp"] = 38;
		entities["lt"] = 60;
		entities["gt"] = 62;
		entities["oelig"] = 338;
		entities["oelig"] = 339;
		entities["scaron"] = 352;
		entities["scaron"] = 353;
		entities["yuml"] = 376;
		entities["circ"] = 710;
		entities["tilde"] = 732;
		entities["ensp"] = 8194;
		entities["emsp"] = 8195;
		entities["thinsp"] = 8201;
		entities["zwnj"] = 8204;
		entities["zwj"] = 8205;
		entities["lrm"] = 8206;
		entities["rlm"] = 8207;
		entities["ndash"] = 8211;
		entities["mdash"] = 8212;
		entities["lsquo"] = 8216;
		entities["rsquo"] = 8217;
		entities["sbquo"] = 8218;
		entities["ldquo"] = 8220;
		entities["rdquo"] = 8221;
		entities["bdquo"] = 8222;
		entities["dagger"] = 8224;
		entities["dagger"] = 8225;
		entities["permil"] = 8240;
		entities["lsaquo"] = 8249;
		entities["rsaquo"] = 8250;
		entities["euro"] = 8364;
		entities["fnof"] = 402;
		entities["alpha"] = 913;
		entities["beta"] = 914;
		entities["gamma"] = 915;
		entities["delta"] = 916;
		entities["epsilon"] = 917;
		entities["zeta"] = 918;
		entities["eta"] = 919;
		entities["theta"] = 920;
		entities["iota"] = 921;
		entities["kappa"] = 922;
		entities["lambda"] = 923;
		entities["mu"] = 924;
		entities["nu"] = 925;
		entities["xi"] = 926;
		entities["omicron"] = 927;
		entities["pi"] = 928;
		entities["rho"] = 929;
		entities["sigma"] = 931;
		entities["tau"] = 932;
		entities["upsilon"] = 933;
		entities["phi"] = 934;
		entities["chi"] = 935;
		entities["psi"] = 936;
		entities["omega"] = 937;
		entities["alpha"] = 945;
		entities["beta"] = 946;
		entities["gamma"] = 947;
		entities["delta"] = 948;
		entities["epsilon"] = 949;
		entities["zeta"] = 950;
		entities["eta"] = 951;
		entities["theta"] = 952;
		entities["iota"] = 953;
		entities["kappa"] = 954;
		entities["lambda"] = 955;
		entities["mu"] = 956;
		entities["nu"] = 957;
		entities["xi"] = 958;
		entities["omicron"] = 959;
		entities["pi"] = 960;
		entities["rho"] = 961;
		entities["sigmaf"] = 962;
		entities["sigma"] = 963;
		entities["tau"] = 964;
		entities["upsilon"] = 965;
		entities["phi"] = 966;
		entities["chi"] = 967;
		entities["psi"] = 968;
		entities["omega"] = 969;
		entities["thetasym"] = 977;
		entities["upsih"] = 978;
		entities["piv"] = 982;
		entities["bull"] = 8226;
		entities["hellip"] = 8230;
		entities["prime"] = 8242;
		entities["prime"] = 8243;
		entities["oline"] = 8254;
		entities["frasl"] = 8260;
		entities["weierp"] = 8472;
		entities["image"] = 8465;
		entities["real"] = 8476;
		entities["trade"] = 8482;
		entities["alefsym"] = 8501;
		entities["larr"] = 8592;
		entities["uarr"] = 8593;
		entities["rarr"] = 8594;
		entities["darr"] = 8595;
		entities["harr"] = 8596;
		entities["crarr"] = 8629;
		entities["larr"] = 8656;
		entities["uarr"] = 8657;
		entities["rarr"] = 8658;
		entities["darr"] = 8659;
		entities["harr"] = 8660;
		entities["forall"] = 8704;
		entities["part"] = 8706;
		entities["exist"] = 8707;
		entities["empty"] = 8709;
		entities["nabla"] = 8711;
		entities["isin"] = 8712;
		entities["notin"] = 8713;
		entities["ni"] = 8715;
		entities["prod"] = 8719;
		entities["sum"] = 8721;
		entities["minus"] = 8722;
		entities["lowast"] = 8727;
		entities["radic"] = 8730;
		entities["prop"] = 8733;
		entities["infin"] = 8734;
		entities["ang"] = 8736;
		entities["and"] = 8743;
		entities["or"] = 8744;
		entities["cap"] = 8745;
		entities["cup"] = 8746;
		entities["int"] = 8747;
		entities["there4"] = 8756;
		entities["sim"] = 8764;
		entities["cong"] = 8773;
		entities["asymp"] = 8776;
		entities["ne"] = 8800;
		entities["equiv"] = 8801;
		entities["le"] = 8804;
		entities["ge"] = 8805;
		entities["sub"] = 8834;
		entities["sup"] = 8835;
		entities["nsub"] = 8836;
		entities["sube"] = 8838;
		entities["supe"] = 8839;
		entities["oplus"] = 8853;
		entities["otimes"] = 8855;
		entities["perp"] = 8869;
		entities["sdot"] = 8901;
		entities["lceil"] = 8968;
		entities["rceil"] = 8969;
		entities["lfloor"] = 8970;
		entities["rfloor"] = 8971;
		entities["lang"] = 9001;
		entities["rang"] = 9002;
		entities["loz"] = 9674;
		entities["spades"] = 9824;
		entities["clubs"] = 9827;
		entities["hearts"] = 9829;
		entities["diams"] = 9830;
		entities["nbsp"] = 160;
		entities["iexcl"] = 161;
		entities["cent"] = 162;
		entities["pound"] = 163;
		entities["curren"] = 164;
		entities["yen"] = 165;
		entities["brvbar"] = 166;
		entities["sect"] = 167;
		entities["uml"] = 168;
		entities["copy"] = 169;
		entities["ordf"] = 170;
		entities["laquo"] = 171;
		entities["not"] = 172;
		entities["shy"] = 173;
		entities["reg"] = 174;
		entities["macr"] = 175;
		entities["deg"] = 176;
		entities["plusmn"] = 177;
		entities["sup2"] = 178;
		entities["sup3"] = 179;
		entities["acute"] = 180;
		entities["micro"] = 181;
		entities["para"] = 182;
		entities["middot"] = 183;
		entities["cedil"] = 184;
		entities["sup1"] = 185;
		entities["ordm"] = 186;
		entities["raquo"] = 187;
		entities["frac14"] = 188;
		entities["frac12"] = 189;
		entities["frac34"] = 190;
		entities["iquest"] = 191;
		entities["agrave"] = 192;
		entities["aacute"] = 193;
		entities["acirc"] = 194;
		entities["atilde"] = 195;
		entities["auml"] = 196;
		entities["aring"] = 197;
		entities["aelig"] = 198;
		entities["ccedil"] = 199;
		entities["egrave"] = 200;
		entities["eacute"] = 201;
		entities["ecirc"] = 202;
		entities["euml"] = 203;
		entities["igrave"] = 204;
		entities["iacute"] = 205;
		entities["icirc"] = 206;
		entities["iuml"] = 207;
		entities["eth"] = 208;
		entities["ntilde"] = 209;
		entities["ograve"] = 210;
		entities["oacute"] = 211;
		entities["ocirc"] = 212;
		entities["otilde"] = 213;
		entities["ouml"] = 214;
		entities["times"] = 215;
		entities["oslash"] = 216;
		entities["ugrave"] = 217;
		entities["uacute"] = 218;
		entities["ucirc"] = 219;
		entities["uuml"] = 220;
		entities["yacute"] = 221;
		entities["thorn"] = 222;
		entities["szlig"] = 223;
		entities["agrave"] = 224;
		entities["aacute"] = 225;
		entities["acirc"] = 226;
		entities["atilde"] = 227;
		entities["auml"] = 228;
		entities["aring"] = 229;
		entities["aelig"] = 230;
		entities["ccedil"] = 231;
		entities["egrave"] = 232;
		entities["eacute"] = 233;
		entities["ecirc"] = 234;
		entities["euml"] = 235;
		entities["igrave"] = 236;
		entities["iacute"] = 237;
		entities["icirc"] = 238;
		entities["iuml"] = 239;
		entities["eth"] = 240;
		entities["ntilde"] = 241;
		entities["ograve"] = 242;
		entities["oacute"] = 243;
		entities["ocirc"] = 244;
		entities["otilde"] = 245;
		entities["ouml"] = 246;
		entities["divide"] = 247;
		entities["oslash"] = 248;
		entities["ugrave"] = 249;
		entities["uacute"] = 250;
		entities["ucirc"] = 251;
		entities["uuml"] = 252;
		entities["yacute"] = 253;
		entities["thorn"] = 254;
		entities["yuml"] = 255;
	}

	return entities;
}

#include "translator.moc"
