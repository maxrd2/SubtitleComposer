/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAVEFORMAT_H
#define WAVEFORMAT_H

#include <QString>

class WaveFormat
{
// valid bits per sample a 8, 16, 24 & 32 (all signed, except 8)

public:
	WaveFormat() :
		m_sampleRate(44100),
		m_channels(2),
		m_bitsPerSample(16),
		m_integer(true)
	{}

	WaveFormat(int sampleRate, int channels, int bitsPerSample, bool integer = true) :
		m_sampleRate(sampleRate),
		m_channels(channels),
		m_bitsPerSample(bitsPerSample),
		m_integer(integer)
	{}

	WaveFormat(const WaveFormat &wf) :
		m_sampleRate(wf.m_sampleRate),
		m_channels(wf.m_channels),
		m_bitsPerSample(wf.m_bitsPerSample),
		m_integer(wf.m_integer)
	{}

	WaveFormat & operator=(const WaveFormat &wf)
	{
		if(this == &wf)
			return *this;
		m_sampleRate = wf.m_sampleRate;
		m_channels = wf.m_channels;
		m_bitsPerSample = wf.m_bitsPerSample;
		m_integer = wf.m_integer;
		return *this;
	}

	bool operator==(const WaveFormat &wf) const { return m_sampleRate == wf.sampleRate() && m_channels == wf.m_channels && m_bitsPerSample == wf.m_bitsPerSample; }
	bool operator!=(const WaveFormat &wf) const { return m_sampleRate != wf.sampleRate() || m_channels != wf.m_channels || m_bitsPerSample != wf.m_bitsPerSample; }

	inline bool isValid() const { return m_sampleRate > 0 && m_channels > 0 && m_bitsPerSample > 0 && (m_bitsPerSample % 8 == 0) && m_bitsPerSample <= 32; }

	inline int sampleRate() const { return m_sampleRate; }
	inline void setSampleRate(int value) { m_sampleRate = value; }

	inline int channels() const { return m_channels; }
	inline void setChannels(int value) { m_channels = value; }

	inline int bitsPerSample() const { return m_bitsPerSample; }
	inline void setBitsPerSample(int value) { m_bitsPerSample = value; }

	inline int bitsPerFrame() const { return m_bitsPerSample * m_channels; }
	inline int bytesPerFrame() const { return m_bitsPerSample * m_channels / 8; }

	inline int bytesPerSample() const { return m_bitsPerSample / 8; }
	inline void setBytesPerSample(int value) { m_bitsPerSample = value * 8; }

	inline bool isSigned() const { return m_bitsPerSample != 8 || !m_integer; }
	inline bool isInteger() const { return m_integer; }
	inline void setInteger(bool value) { m_integer = value; }

	inline int blockAlign() const { return m_channels * m_bitsPerSample / 8; }
	inline static int blockAlign(int channels, int bitsPerSample) { return channels * bitsPerSample / 8; }

	inline QString toString() const { return QStringLiteral("%1Hz.%2ch.%3bps.%4").arg(m_sampleRate).arg(m_channels).arg(m_bitsPerSample).arg(m_integer ? "int" : "float"); }

private:
	int m_sampleRate;
	int m_channels;
	int m_bitsPerSample;
	bool m_integer;
};

#endif
