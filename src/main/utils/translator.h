#ifndef TRANSLATOR_H
#define TRANSLATOR_H

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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "language.h"
#include "../dialogs/progressdialog.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QByteArray>

class KJob;
namespace KIO
{
	class Job;
	class TransferJob;
}

namespace SubtitleComposer
{
	class Translator : public QObject
	{
		Q_OBJECT

		public:

			static const int MaxChunkSize = 25000; // in characters

			Translator( QObject* parent=0 );
			~Translator();

			QString inputText() const;
			QString outputText() const;

			Language::Value inputLanguage() const;
			Language::Value outputLanguage() const;

			int chunksCount() const; // upon how many chunks will the imput text be splitted?

			bool isFinished() const;
			bool isFinishedWithError() const;
			bool isAborted() const;

			QString errorMessage() const;

		public slots:

			bool syncTranslate( const QString& text, Language::Value inputLang, Language::Value outputLang, ProgressDialog*pd=0 );
			void translate( const QString& text, Language::Value inputLang, Language::Value outputLang );

			void abort();

		signals:

			void progress( int percentage );

			void finished( const QString& translatedText );
			void finishedWithError( const QString& errorMessage );

			void finished(); // finished with or without error

		private:

			static QByteArray prepareUrlEncodedData( const QMap<QString,QString>& params );
			static QByteArray prepareMultipartData( const QMap<QString,QString>& params );
			static QString& replaceHTMLEntities( QString& text );
			static const QMap<QString, QChar>& namedEntities();

			void startChunkDownload( int chunkNumber ); // first chunk is number 1

		private slots:

			void onTransferJobProgress( KJob* job, unsigned long percent );
			void onTransferJobData( KIO::Job* job, const QByteArray& data );
			void onTransferJobResult( KJob* job );

		private:

			KIO::TransferJob* m_currentTransferJob;
			QByteArray m_currentTransferData;
			QStringList m_inputTextChunks;
			QString m_outputText;
			Language::Value m_inputLanguage;
			Language::Value m_outputLanguage;
			int m_lastReceivedChunk;
			QString m_errorMessage;
			bool m_aborted;
	};
}

#endif
