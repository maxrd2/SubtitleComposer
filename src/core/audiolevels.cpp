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

#include "audiolevels.h"
#include "../common/fileloadhelper.h"
#include "../common/filesavehelper.h"

#include <vector>
#include <cmath>                                // FIXME remove when unneeded

#include <QtCore/QFile>
#include <QtCore/QTextStream>

using namespace SubtitleComposer;

AudioLevels::AudioLevels() :
	m_channelsCount(0),
	m_samplesInterval(50),
	m_samplesCount(0),
	m_samplesData(0),
	m_lastDataIndex(-1),
	m_mediaUrl()
{}

AudioLevels::~AudioLevels()
{
	delete[] m_samplesData;
}

bool
AudioLevels::load(const KUrl &fileUrl)
{
	if(m_samplesData)
		return false;

	FileLoadHelper fileLoadHelper(fileUrl);

	if(!fileLoadHelper.open())
		return false;

	QFile *file = fileLoadHelper.file();

	QTextStream textStream(file);

	if(QString("audiolevels") != textStream.readLine())
		return false;

	m_mediaUrl = KUrl(textStream.readLine());

	file->read((char *)&m_channelsCount, sizeof(m_channelsCount));

	qint64 duration;
	file->read((char *)&duration, sizeof(duration));

	m_samplesCount = 0;
	for(unsigned channel = 0; channel < m_channelsCount; ++channel) {
		std::vector<double>::size_type samplesCount;
		file->read((char *)&samplesCount, sizeof(samplesCount));

		if(!m_samplesCount) {
			m_samplesCount = samplesCount;
			m_samplesData = new double[m_samplesCount * m_channelsCount];
			m_lastDataIndex = m_samplesCount * m_channelsCount - 1;
		} else if(m_samplesCount != samplesCount) {
			reset();
			file->close();
			return false;
		}

		file->read((char *)&m_samplesData[m_samplesCount * channel], sizeof(double) * m_samplesCount);
	}

	m_samplesInterval = (int)(duration / 1000000.0 / m_samplesCount);

	if(!m_samplesInterval) {
		reset();
		return false;
	}

	return true;
}

bool
AudioLevels::loadFromMedia(const QString & /*mediaPath */, unsigned /*streamIndex */)
{
	// STUB IMPLEMENTATION just to get something we can use

	m_channelsCount = 1;
	m_samplesCount = 30000;
	m_samplesData = new double[m_samplesCount * m_channelsCount];
	m_lastDataIndex = m_samplesCount * m_channelsCount - 1;

	double degrees = 0;
	for(int idx = 0; idx < 30000; ++idx) {
		m_samplesData[idx] = sin(degrees * M_PI / 180) / 100;
		degrees += 10.0;
	}
	m_samplesInterval = 100;

	return true;

	return false; // FIXME: stub implementation
}

bool
AudioLevels::save(const KUrl &fileUrl, bool overwrite) const
{
	FileSaveHelper fileSaveHelper(fileUrl, overwrite);

	if(!fileSaveHelper.open())
		return false;

	QFile *file = fileSaveHelper.file();

	QString audiolevels("audiolevels\n");
	file->write(audiolevels.toAscii(), audiolevels.length());

	QString url = m_mediaUrl.url() + '\n';
	file->write(url.toAscii(), url.length());

	file->write((const char *)&m_channelsCount, sizeof(m_channelsCount));

	qint64 duration = (qint64)m_samplesInterval * (qint64)m_samplesCount * 1000000;
	file->write((const char *)&duration, sizeof(duration));

	for(unsigned channel = 0; channel < m_channelsCount; ++channel) {
		std::vector<double>::size_type samplesCount = m_samplesCount;
		file->write((const char *)&samplesCount, sizeof(samplesCount));

		file->write((const char *)&m_samplesData[m_samplesCount * channel], sizeof(double) * m_samplesCount);
	}
	return fileSaveHelper.close();
}

void
AudioLevels::reset()
{
	m_channelsCount = 0;
	m_samplesInterval = 50;
	m_samplesCount = 0;
	delete[] m_samplesData;
	m_samplesData = 0;
	m_lastDataIndex = -1;
	m_mediaUrl = KUrl();
}

#include "audiolevels.moc"
