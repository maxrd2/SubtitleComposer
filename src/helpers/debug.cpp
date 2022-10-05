/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "debug.h"

#include "helpers/common.h"

#include <QStringBuilder>

using namespace SubtitleComposer;


QString
SubtitleComposer::propertyName(QTextFormat::Property key)
{
	static const QMap<int, QString> names = {
		{ QTextFormat::ObjectIndex, $("ObjectIndex") },

		// paragraph and char
		{ QTextFormat::CssFloat, $("CssFloat") },
		{ QTextFormat::LayoutDirection, $("LayoutDirection") },

		{ QTextFormat::OutlinePen, $("OutlinePen") },
		{ QTextFormat::BackgroundBrush, $("BackgroundBrush") },
		{ QTextFormat::ForegroundBrush, $("ForegroundBrush") },
		// Internal to qtextlayout.cpp: ObjectSelectionBrush
		{ QTextFormat::BackgroundImageUrl, $("BackgroundImageUrl") },

		// paragraph
		{ QTextFormat::BlockAlignment, $("BlockAlignment") },
		{ QTextFormat::BlockTopMargin, $("BlockTopMargin") },
		{ QTextFormat::BlockBottomMargin, $("BlockBottomMargin") },
		{ QTextFormat::BlockLeftMargin, $("BlockLeftMargin") },
		{ QTextFormat::BlockRightMargin, $("BlockRightMargin") },
		{ QTextFormat::TextIndent, $("TextIndent") },
		{ QTextFormat::TabPositions, $("TabPositions") },
		{ QTextFormat::BlockIndent, $("BlockIndent") },
		{ QTextFormat::LineHeight, $("LineHeight") },
		{ QTextFormat::LineHeightType, $("LineHeightType") },
		{ QTextFormat::BlockNonBreakableLines, $("BlockNonBreakableLines") },
		{ QTextFormat::BlockTrailingHorizontalRulerWidth, $("BlockTrailingHorizontalRulerWidth") },
		{ QTextFormat::HeadingLevel, $("HeadingLevel") },
		{ QTextFormat::BlockQuoteLevel, $("BlockQuoteLevel") },
		{ QTextFormat::BlockCodeLanguage, $("BlockCodeLanguage") },
		{ QTextFormat::BlockCodeFence, $("BlockCodeFence") },
		{ QTextFormat::BlockMarker, $("BlockMarker") },

		// character properties
		{ QTextFormat::FirstFontProperty, $("FirstFontProperty") },
		{ QTextFormat::FontCapitalization, $("FontCapitalization") },
		{ QTextFormat::FontLetterSpacingType, $("FontLetterSpacingType") },
		{ QTextFormat::FontLetterSpacing, $("FontLetterSpacing") },
		{ QTextFormat::FontWordSpacing, $("FontWordSpacing") },
		{ QTextFormat::FontStretch, $("FontStretch") },
		{ QTextFormat::FontStyleHint, $("FontStyleHint") },
		{ QTextFormat::FontStyleStrategy, $("FontStyleStrategy") },
		{ QTextFormat::FontKerning, $("FontKerning") },
		{ QTextFormat::FontHintingPreference, $("FontHintingPreference") },
		{ QTextFormat::FontFamilies, $("FontFamilies") },
		{ QTextFormat::FontStyleName, $("FontStyleName") },
		{ QTextFormat::FontFamily, $("FontFamily") },
		{ QTextFormat::FontPointSize, $("FontPointSize") },
		{ QTextFormat::FontSizeAdjustment, $("FontSizeAdjustment") },
		{ QTextFormat::FontSizeIncrement, $("FontSizeIncrement") },
		{ QTextFormat::FontWeight, $("FontWeight") },
		{ QTextFormat::FontItalic, $("FontItalic") },
		{ QTextFormat::FontUnderline, $("FontUnderline") },
		{ QTextFormat::FontOverline, $("FontOverline") },
		{ QTextFormat::FontStrikeOut, $("FontStrikeOut") },
		{ QTextFormat::FontFixedPitch, $("FontFixedPitch") },
		{ QTextFormat::FontPixelSize, $("FontPixelSize") },
		{ QTextFormat::LastFontProperty, $("LastFontProperty") },

		{ QTextFormat::TextUnderlineColor, $("TextUnderlineColor") },
		{ QTextFormat::TextVerticalAlignment, $("TextVerticalAlignment") },
		{ QTextFormat::TextOutline, $("TextOutline") },
		{ QTextFormat::TextUnderlineStyle, $("TextUnderlineStyle") },
		{ QTextFormat::TextToolTip, $("TextToolTip") },

		{ QTextFormat::IsAnchor, $("IsAnchor") },
		{ QTextFormat::AnchorHref, $("AnchorHref") },
		{ QTextFormat::AnchorName, $("AnchorName") },
		{ QTextFormat::ObjectType, $("ObjectType") },

		// list properties
		{ QTextFormat::ListStyle, $("ListStyle") },
		{ QTextFormat::ListIndent, $("ListIndent") },
		{ QTextFormat::ListNumberPrefix, $("ListNumberPrefix") },
		{ QTextFormat::ListNumberSuffix, $("ListNumberSuffix") },

		// table and frame properties
		{ QTextFormat::FrameBorder, $("FrameBorder") },
		{ QTextFormat::FrameMargin, $("FrameMargin") },
		{ QTextFormat::FramePadding, $("FramePadding") },
		{ QTextFormat::FrameWidth, $("FrameWidth") },
		{ QTextFormat::FrameHeight, $("FrameHeight") },
		{ QTextFormat::FrameTopMargin, $("FrameTopMargin") },
		{ QTextFormat::FrameBottomMargin, $("FrameBottomMargin") },
		{ QTextFormat::FrameLeftMargin, $("FrameLeftMargin") },
		{ QTextFormat::FrameRightMargin, $("FrameRightMargin") },
		{ QTextFormat::FrameBorderBrush, $("FrameBorderBrush") },
		{ QTextFormat::FrameBorderStyle, $("FrameBorderStyle") },

		{ QTextFormat::TableColumns, $("TableColumns") },
		{ QTextFormat::TableColumnWidthConstraints, $("TableColumnWidthConstraints") },
		{ QTextFormat::TableCellSpacing, $("TableCellSpacing") },
		{ QTextFormat::TableCellPadding, $("TableCellPadding") },
		{ QTextFormat::TableHeaderRowCount, $("TableHeaderRowCount") },
		{ QTextFormat::TableBorderCollapse, $("TableBorderCollapse") },

		// table cell properties
		{ QTextFormat::TableCellRowSpan, $("TableCellRowSpan") },
		{ QTextFormat::TableCellColumnSpan, $("TableCellColumnSpan") },

		{ QTextFormat::TableCellTopPadding, $("TableCellTopPadding") },
		{ QTextFormat::TableCellBottomPadding, $("TableCellBottomPadding") },
		{ QTextFormat::TableCellLeftPadding, $("TableCellLeftPadding") },
		{ QTextFormat::TableCellRightPadding, $("TableCellRightPadding") },

		{ QTextFormat::TableCellTopBorder, $("TableCellTopBorder") },
		{ QTextFormat::TableCellBottomBorder, $("TableCellBottomBorder") },
		{ QTextFormat::TableCellLeftBorder, $("TableCellLeftBorder") },
		{ QTextFormat::TableCellRightBorder, $("TableCellRightBorder") },

		{ QTextFormat::TableCellTopBorderStyle, $("TableCellTopBorderStyle") },
		{ QTextFormat::TableCellBottomBorderStyle, $("TableCellBottomBorderStyle") },
		{ QTextFormat::TableCellLeftBorderStyle, $("TableCellLeftBorderStyle") },
		{ QTextFormat::TableCellRightBorderStyle, $("TableCellRightBorderStyle") },

		{ QTextFormat::TableCellTopBorderBrush, $("TableCellTopBorderBrush") },
		{ QTextFormat::TableCellBottomBorderBrush, $("TableCellBottomBorderBrush") },
		{ QTextFormat::TableCellLeftBorderBrush, $("TableCellLeftBorderBrush") },
		{ QTextFormat::TableCellRightBorderBrush, $("TableCellRightBorderBrush") },

		// image properties
		{ QTextFormat::ImageName, $("ImageName") },
		{ QTextFormat::ImageTitle, $("ImageTitle") },
		{ QTextFormat::ImageAltText, $("ImageAltText") },
		{ QTextFormat::ImageWidth, $("ImageWidth") },
		{ QTextFormat::ImageHeight, $("ImageHeight") },
		{ QTextFormat::ImageQuality, $("ImageQuality") },

		// internal
		{ 0x5012/*QTextFormat::SuppressText*/, $("SuppressText") },
		{ 0x5013/*QTextFormat::SuppressBackground*/, $("SuppressBackground") },
		{ 0x513/*QTextFormat::SuppressBackground*/, $("SuppressBackground") },

		// selection properties
		{ QTextFormat::FullWidthSelection, $("FullWidthSelection") },

		// page break properties
		{ QTextFormat::PageBreakPolicy, $("PageBreakPolicy") },

		// --
		{ QTextFormat::UserProperty, $("UserProperty") },
	};
	if(key >= QTextFormat::UserProperty)
		return $("UserProperty+") % QString::number(key - QTextFormat::UserProperty);
	const auto it = names.find(key);
	return it != names.cend() ? it.value() : QString::number(key);
}

QString
SubtitleComposer::textFormatString(const QTextFormat &format)
{
	QString res;
	const QMap<int, QVariant> &props = format.properties();
	for(auto it = props.cbegin(); it != props.cend(); ++it)
		res.append(propertyName(QTextFormat::Property(it.key())) % QChar(':') % it.value().toString() % $("; "));
	return res;
}

QString
SubtitleComposer::dumpFormatRanges(const QString &text, const QVector<QTextLayout::FormatRange> &ranges)
{
	QString res;
	for(const QTextLayout::FormatRange &r: ranges) {
		res.append(QChar::LineFeed % QChar('"') % text.midRef(r.start, r.length) % QChar('"') % QChar::Space % textFormatString(r.format));
	}
	return res;
}
