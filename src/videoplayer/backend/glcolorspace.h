/*
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QMap>
#include <QVector>
#include <QOpenGLFunctions>

namespace SubtitleComposer {

// from ISO IEC 23001-8:2018 (7.1) data
const static QMap<int, QVector<GLfloat>> _csm{
	// 0 - RESERVED - For future use by ISO/IEC
	// 1 - ITU-R BT.709-5; ITU-R BT.1361; IEC 61966-2-1 sRGB or sYCC; IEC 61966-2-4; SMPTE-RP-177:1993b
	{ 1, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1872813f, 1.85564f, 1.574726f, -0.4681946f, 0.0f }},
	// 2 - UNSPECIFIED - Image characteristics are unknown or are determined by the application
	// 3 - RESERVED - For future use by ISO/IEC
	// 4 - ITU-R BT.470-6m; US-NTSC-1953; USFCCT-47:2003-73.682a
	{ 4, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.3456142f, 1.771046f, 1.402194f, -0.7144662f, 0.0f }},
	// 5 - ITU-R BT.470-6bg; ITU-R BT.601-6 625; ITU-R BT.1358 625; ITU-R BT.1700 625 PAL/SECAM
	{ 5, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1874739f, 1.857342f, 1.555995f, -0.488821f, 0.0f }},
	// 6 - ITU-R BT.601-6 525; ITU-R BT.1358 525; ITU-R BT.1700 NTSC; SMPTE-170M:2004
	{ 6, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.2255347f, 1.826901f, 1.575252f, -0.4771844f, 0.0f }},
	// 7 - SMPTE-240M:1999
	{ 7, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.2255347f, 1.826901f, 1.575252f, -0.4771844f, 0.0f }},
	// 8 - Generic film (color filters using CIE SI C)
	{ 8, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1868185f, 1.864001f, 1.492941f, -0.557879f, 0.0f }},
	// 9 - Rec. ITU-R BT.2020
	{ 9, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1645325f, 1.881414f, 1.474603f, -0.5713434f, 0.0f }},
	// 10 - SMPTE-ST-428-1 (CIE 1931 XYZ as in ISO 11664-1)
	{ 10, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.0f, 0.0f }},
	// 11 - SMPTE-RP-431-2:2011
	{ 11, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1778395f, 1.862174f, 1.581017f, -0.4589967f, 0.0f }},
	// 12 - SMPTE-EG-432-1:2010
	{ 12, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.2110306f, 1.84145f, 1.542053f, -0.5104277f, 0.0f }},
	// 13 - RESERVED - For future use by ISO/IEC
	// 14 - RESERVED - For future use by ISO/IEC
	// 15 - RESERVED - For future use by ISO/IEC
	// 16 - RESERVED - For future use by ISO/IEC
	// 17 - RESERVED - For future use by ISO/IEC
	// 18 - RESERVED - For future use by ISO/IEC
	// 19 - RESERVED - For future use by ISO/IEC
	// 20 - RESERVED - For future use by ISO/IEC
	// 21 - RESERVED - For future use by ISO/IEC
	// 22 - EBU-3213-E:1975
	{ 22, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.2581406f, 1.808035f, 1.536503f, -0.5296722f, 0.0f }},
};

// from ISO IEC 23001-8:2018 (7.2) data
const static QMap<int, QString> _ctf{
	// 0 - RESERVED - For future use by ISO/IEC
	// 1 - ITU-R BT.709-5; ITU-R BT.1361
	{ 1, QStringLiteral("if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.45) - 0.09929683;") },
	// 2 - Unspecified - Image characteristics are unknown or are determined by the application
	// 3 - RESERVED - For future use by ISO/IEC
	// 4 - ITU-R BT.470-6m; US-NTSC-1953; USFCCT-47:2003-73.682a; ITU-R BT.1700:2007 625 PAL/SECAM
	{ 4, QStringLiteral("if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.4545455) - 0.09929683;") },
	// 5 - ITU-R BT.1700:2007 625 PAL/SECAM; ITU-R BT.470-6bg
	{ 5, QStringLiteral("if(vLin < 0.0031308) return 12.92 * vLin;"
			"return 1.055 * pow(vLin, 0.3571429) - 0.055;") },
	// 6 - ITU-R BT.601-6 525/625; ITU-R BT.1358 525/625; ITU-R BT.1700 NTSC; SMPTE-170M:2004
	{ 6, QStringLiteral("if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.45) - 0.09929683;") },
	// 7 - SMPTE-240M:1999
	{ 7, QStringLiteral("if(vLin < 0.0228) return 4.0 * vLin;"
			"return 1.1115 * pow(vLin, 0.45) - 0.1115;") },
	// 8 - Linear transfer characteristics
	{ 8, QStringLiteral("return vLin;") },
	// 9 - Logarithmic transfer characteristic (100:1 range)
	{ 9, QStringLiteral("if(vLin < 0.01) return 0.0;"
			"return 1.0 + (log(vLin) / log(10.0)) / 2.0;") },
	// 10 - Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)
	{ 10, QStringLiteral("if(vLin < 0.003162278) return 0.0;"
			"return 1.0 + (log(vLin) / log(10.0)) / 2.5;") },
	// 11 - IEC 61966-2-4
	{ 11, QStringLiteral("if(vLin < -0.01805397) return -1.099297 * pow(-vLin, 0.45) + 0.09929683;"
			"if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.45) - 0.09929683;") },
	// 12 - ITU-R BT.1361
	{ 12, QStringLiteral("if(vLin < -0.004) return (-(1.099297 * pow(-4.0 * vLin, 0.45) - 0.09929683)) / 4.0;"
			"if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.45) - 0.09929683;") },
	// 13 - IEC 61966-2-1 sRGB/sYCC
	{ 13, QStringLiteral("if(vLin < 0.0031308) return 12.92 * vLin;"
			"return 1.055 * pow(vLin, 0.4166667) - 0.055;") },
	// 14 - ITU-R BT.2020 (10-bit system)
	{ 14, QStringLiteral("if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.45) - 0.09929683;") },
	// 15 - ITU-R BT.2020 (12-bit system)
	{ 15, QStringLiteral("if(vLin < 0.01805397) return 4.5 * vLin;"
			"return 1.099297 * pow(vLin, 0.45) - 0.09929683;") },
	// 16 - SMPTE-ST-2084 (for TV 10, 12, 14, and 16-bit systems)
	{ 16, QStringLiteral("return pow((-1.164063 + 18.85156 * pow(vLin, 0.1594238)) / (1.0 + 18.6875 * pow(vLin, 0.1594238)), 78.84375);") },
	// 17 - SMPTE-ST-428-1
	{ 17, QStringLiteral("return pow((48.0 * vLin) / 52.37, 0.3846154);") },
};
const static QMap<int, QString> _ctfi{
	// 0 - RESERVED - For future use by ISO/IEC
	// 1 - ITU-R BT.709-5; ITU-R BT.1361
	{ 1, QStringLiteral("if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.222222);") },
	// 2 - Unspecified - Image characteristics are unknown or are determined by the application
	// 3 - RESERVED - For future use by ISO/IEC
	// 4 - ITU-R BT.470-6m; US-NTSC-1953; USFCCT-47:2003-73.682a; ITU-R BT.1700:2007 625 PAL/SECAM
	{ 4, QStringLiteral("if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.2);") },
	// 5 - ITU-R BT.1700:2007 625 PAL/SECAM; ITU-R BT.470-6bg
	{ 5, QStringLiteral("if(vExp < 0.04044994) return vExp / 12.92;"
			"return pow((vExp + 0.055) / 1.055, 2.8);") },
	// 6 - ITU-R BT.601-6 525/625; ITU-R BT.1358 525/625; ITU-R BT.1700 NTSC; SMPTE-170M:2004
	{ 6, QStringLiteral("if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.222222);") },
	// 7 - SMPTE-240M:1999
	{ 7, QStringLiteral("if(vExp < 0.0912) return vExp / 4.0;"
			"return pow((vExp + 0.1115) / 1.1115, 2.222222);") },
	// 8 - Linear transfer characteristics
	{ 8, QStringLiteral("return vExp;") },
	// 9 - Logarithmic transfer characteristic (100:1 range)
	{ 9, QStringLiteral("return pow(10.0, (vExp - 1.0) * 2.0);") },
	// 10 - Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)
	{ 10, QStringLiteral("return pow(10.0, (vExp - 1.0) * 2.5);") },
	// 11 - IEC 61966-2-4
	{ 11, QStringLiteral("if(vExp < -0.08124286) return -pow((vExp - 0.09929683) / -1.099297, 2.222222);"
			"if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.222222);") },
	// 12 - ITU-R BT.1361
	{ 12, QStringLiteral("if(vExp < -0.01792312) return pow((-4.0 * vExp + 0.09929683) / 1.099297, 2.222222) / -4.0;"
			"if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.222222);") },
	// 13 - IEC 61966-2-1 sRGB/sYCC
	{ 13, QStringLiteral("if(vExp < 0.04044994) return vExp / 12.92;"
			"return pow((vExp + 0.055) / 1.055, 2.4);") },
	// 14 - ITU-R BT.2020 (10-bit system)
	{ 14, QStringLiteral("if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.222222);") },
	// 15 - ITU-R BT.2020 (12-bit system)
	{ 15, QStringLiteral("if(vExp < 0.08124286) return vExp / 4.5;"
			"return pow((vExp + 0.09929683) / 1.099297, 2.222222);") },
	// 16 - SMPTE-ST-2084 (for TV 10, 12, 14, and 16-bit systems)
	{ 16, QStringLiteral("return pow((pow(vExp, 0.01268331) * (1.0 + 18.6875 * pow(vLin, 0.1594238)) + 1.164063) / 18.85156, 6.272588);") },
	// 17 - SMPTE-ST-428-1
	{ 17, QStringLiteral("return pow(vExp, 2.6) * 1.091042;") },
};
// from ISO IEC 23001-8:2018 (7.3) data
const static QMap<int, QVector<GLfloat>> _csc{
	// 0 - The identity matrix (RGB/XYZ); IEC 61966-2-1 sRGB; SMPTE-ST-428-1; ITU-R BT.709-5
	{ 0, QVector<GLfloat>{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }},
	// 1 - ITU-R BT.709-5; ITU-R BT.1361; IEC 61966-2-1/4 sYCC/xvYCC709; SMPTE-RP-177:1993b
	{ 1, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1873243f, 1.8556f, 1.5748f, -0.4681243f, 0.0f }},
	// 2 - UNSPECIFIED - Image characteristics are unknown or are determined by the application
	// 3 - RESERVED - For future use by ISO/IEC
	// 4 - USFCCT-47:2003-73.682a
	{ 4, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.3345051f, 1.778f, 1.4f, -0.7118644f, 0.0f }},
	// 5 - ITU-R BT.470-6bg; ITU-R BT.601-6 625; ITU-R BT.1358 625; ITU-R BT.1700 625 PAL/SECAM; IEC 61966-2-4 xvYCC601
	{ 5, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.3441363f, 1.772f, 1.402f, -0.7141363f, 0.0f }},
	// 6 - ITU-R BT.601-6 525; ITU-R BT.1358 525; ITU-R BT.1700 NTSC; SMPTE-170M:2004
	{ 6, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.3441363f, 1.772f, 1.402f, -0.7141363f, 0.0f }},
	// 7 - SMPTE-240M:1999
	{ 7, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.226622f, 1.826f, 1.576f, -0.476622f, 0.0f }},
	// 8 - ITU-T SG16; Dirac/VC-2 and H.264 FRext
	{ 8, QVector<GLfloat>{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }},
	// 9 - ITU-R BT.2020 (non-constant luminance)
	{ 9, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1645531f, 1.8814f, 1.4746f, -0.5713531f, 0.0f }},
	// 10 - ITU-R BT.2020 (constant luminance)
	{ 10, QVector<GLfloat>{ 1.0f, 1.0f, 1.0f, 0.0f, -0.1645531f, 1.8814f, 1.4746f, -0.5713531f, 0.0f }},
	// 11 - SMPTE-ST-2085:2015
	{ 11, QVector<GLfloat>{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }},
	// 12 - TODO: AVCOL_SPC_CHROMA_DERIVED_NCL defined in FFmpeg?
	// 13 - TODO: AVCOL_SPC_CHROMA_DERIVED_CL defined in FFmpeg?
	// 14 - TODO: AVCOL_SPC_ICTCP defined in FFmpeg?
};


}
