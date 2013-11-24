#ifndef PROFILER_H
#define PROFILER_H

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

#define PROFILING

#ifdef PROFILING

#include <QtCore/QTime>
#include <KDebug>

class Profiler
{
public:
	Profiler(const char *description = 0) :
		m_description(description)
	{
		m_time.start();
	}

	~Profiler()
	{
		int elapsed = m_time.elapsed();
		if(m_description)
			qDebug() << m_description << " took" << elapsed << "msecs";
		else
			qDebug() << "took" << elapsed << "msecs";
	}

private:
	const char *m_description;
	QTime m_time;
};

#define PROFILE() Profiler _p_r_o_f_i_l_e_r_(Q_FUNC_INFO)
#define PROFILE2(x) Profiler _p_r_o_f_i_l_e_r_(x)

#else

#define PROFILE() ;
#define PROFILE2(x) ;

#endif

#endif
