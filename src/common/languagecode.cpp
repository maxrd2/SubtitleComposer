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

#include "languagecode.h"

#include <QtCore/QMap>

#include <KGlobal>
#include <KLocale>

QString
LanguageCode::toIso2(const QString &iso3Code)
{
	static QMap<QString, QString> map;
	if(map.isEmpty()) {
		map["aar"] = "aa";
		map["abk"] = "ab";
		map["afr"] = "af";
		map["aka"] = "ak";
		map["alb"] = "sq";
		map["amh"] = "am";
		map["ara"] = "ar";
		map["arg"] = "an";
		map["arm"] = "hy";
		map["asm"] = "as";
		map["ava"] = "av";
		map["ave"] = "ae";
		map["aym"] = "ay";
		map["aze"] = "az";
		map["bak"] = "ba";
		map["bam"] = "bm";
		map["baq"] = "eu";
		map["bel"] = "be";
		map["ben"] = "bn";
		map["bih"] = "bh";
		map["bis"] = "bi";
		map["bos"] = "bs";
		map["bre"] = "br";
		map["bul"] = "bg";
		map["bur"] = "my";
		map["cat"] = "ca";
		map["cha"] = "ch";
		map["che"] = "ce";
		map["chi"] = "zh";
		map["chu"] = "cu";
		map["chv"] = "cv";
		map["cor"] = "kw";
		map["cos"] = "co";
		map["cre"] = "cr";
		map["cze"] = "cs";
		map["dan"] = "da";
		map["div"] = "dv";
		map["dut"] = "nl";
		map["dzo"] = "dz";
		map["eng"] = "en";
		map["epo"] = "eo";
		map["est"] = "et";
		map["ewe"] = "ee";
		map["fao"] = "fo";
		map["fij"] = "fj";
		map["fin"] = "fi";
		map["fre"] = "fr";
		map["fry"] = "fy";
		map["ful"] = "ff";
		map["geo"] = "ka";
		map["ger"] = "de";
		map["gla"] = "gd";
		map["gle"] = "ga";
		map["glg"] = "gl";
		map["glv"] = "gv";
		map["gre"] = "el";
		map["grn"] = "gn";
		map["guj"] = "gu";
		map["hat"] = "ht";
		map["hau"] = "ha";
		map["heb"] = "he";
		map["her"] = "hz";
		map["hin"] = "hi";
		map["hmo"] = "ho";
		map["hrv"] = "hr";
		map["hun"] = "hu";
		map["ibo"] = "ig";
		map["ice"] = "is";
		map["ido"] = "io";
		map["iii"] = "ii";
		map["iku"] = "iu";
		map["ile"] = "ie";
		map["ina"] = "ia";
		map["ind"] = "id";
		map["ipk"] = "ik";
		map["ita"] = "it";
		map["jav"] = "jv";
		map["jpn"] = "ja";
		map["kal"] = "kl";
		map["kan"] = "kn";
		map["kas"] = "ks";
		map["kau"] = "kr";
		map["kaz"] = "kk";
		map["khm"] = "km";
		map["kik"] = "ki";
		map["kin"] = "rw";
		map["kir"] = "ky";
		map["kom"] = "kv";
		map["kon"] = "kg";
		map["kor"] = "ko";
		map["kua"] = "kj";
		map["kur"] = "ku";
		map["lao"] = "lo";
		map["lat"] = "la";
		map["lav"] = "lv";
		map["lim"] = "li";
		map["lin"] = "ln";
		map["lit"] = "lt";
		map["ltz"] = "lb";
		map["lub"] = "lu";
		map["lug"] = "lg";
		map["mac"] = "mk";
		map["mah"] = "mh";
		map["mal"] = "ml";
		map["mao"] = "mi";
		map["mar"] = "mr";
		map["may"] = "ms";
		map["mlg"] = "mg";
		map["mlt"] = "mt";
		map["mon"] = "mn";
		map["nau"] = "na";
		map["nav"] = "nv";
		map["nbl"] = "nr";
		map["nde"] = "nd";
		map["ndo"] = "ng";
		map["nep"] = "ne";
		map["nno"] = "nn";
		map["nob"] = "nb";
		map["nor"] = "no";
		map["nya"] = "ny";
		map["oci"] = "oc";
		map["oji"] = "oj";
		map["ori"] = "or";
		map["orm"] = "om";
		map["oss"] = "os";
		map["pan"] = "pa";
		map["per"] = "fa";
		map["pli"] = "pi";
		map["pol"] = "pl";
		map["por"] = "pt";
		map["pus"] = "ps";
		map["que"] = "qu";
		map["roh"] = "rm";
		map["rum"] = "ro";
		map["run"] = "rn";
		map["rus"] = "ru";
		map["sag"] = "sg";
		map["san"] = "sa";
		map["sin"] = "si";
		map["slo"] = "sk";
		map["slv"] = "sl";
		map["sme"] = "se";
		map["smo"] = "sm";
		map["sna"] = "sn";
		map["snd"] = "sd";
		map["som"] = "so";
		map["sot"] = "st";
		map["spa"] = "es";
		map["srd"] = "sc";
		map["srp"] = "sr";
		map["ssw"] = "ss";
		map["sun"] = "su";
		map["swa"] = "sw";
		map["swe"] = "sv";
		map["tah"] = "ty";
		map["tam"] = "ta";
		map["tat"] = "tt";
		map["tel"] = "te";
		map["tgk"] = "tg";
		map["tgl"] = "tl";
		map["tha"] = "th";
		map["tib"] = "bo";
		map["tir"] = "ti";
		map["ton"] = "to";
		map["tsn"] = "tn";
		map["tso"] = "ts";
		map["tuk"] = "tk";
		map["tur"] = "tr";
		map["twi"] = "tw";
		map["uig"] = "ug";
		map["ukr"] = "uk";
		map["urd"] = "ur";
		map["uzb"] = "uz";
		map["ven"] = "ve";
		map["vie"] = "vi";
		map["vol"] = "vo";
		map["wel"] = "cy";
		map["wln"] = "wa";
		map["wol"] = "wo";
		map["xho"] = "xh";
		map["yid"] = "yi";
		map["yor"] = "yo";
		map["zha"] = "za";
		map["zul"] = "zu";
	}

	return map.contains(iso3Code) ? map[iso3Code] : QString();
}

QString
LanguageCode::toIso3(const QString &iso2Code)
{
	static QMap<QString, QString> map;
	if(map.isEmpty()) {
		map["aa"] = "aar";
		map["ab"] = "abk";
		map["ae"] = "ave";
		map["af"] = "afr";
		map["ak"] = "aka";
		map["am"] = "amh";
		map["an"] = "arg";
		map["ar"] = "ara";
		map["as"] = "asm";
		map["av"] = "ava";
		map["ay"] = "aym";
		map["az"] = "aze";
		map["ba"] = "bak";
		map["be"] = "bel";
		map["bg"] = "bul";
		map["bh"] = "bih";
		map["bi"] = "bis";
		map["bm"] = "bam";
		map["bn"] = "ben";
		map["bo"] = "tib";
		map["br"] = "bre";
		map["bs"] = "bos";
		map["ca"] = "cat";
		map["ce"] = "che";
		map["ch"] = "cha";
		map["co"] = "cos";
		map["cr"] = "cre";
		map["cs"] = "cze";
		map["cu"] = "chu";
		map["cv"] = "chv";
		map["cy"] = "wel";
		map["da"] = "dan";
		map["de"] = "ger";
		map["dv"] = "div";
		map["dz"] = "dzo";
		map["ee"] = "ewe";
		map["el"] = "gre";
		map["en"] = "eng";
		map["eo"] = "epo";
		map["es"] = "spa";
		map["et"] = "est";
		map["eu"] = "baq";
		map["fa"] = "per";
		map["ff"] = "ful";
		map["fi"] = "fin";
		map["fj"] = "fij";
		map["fo"] = "fao";
		map["fr"] = "fre";
		map["fy"] = "fry";
		map["ga"] = "gle";
		map["gd"] = "gla";
		map["gl"] = "glg";
		map["gn"] = "grn";
		map["gu"] = "guj";
		map["gv"] = "glv";
		map["ha"] = "hau";
		map["he"] = "heb";
		map["hi"] = "hin";
		map["ho"] = "hmo";
		map["hr"] = "hrv";
		map["ht"] = "hat";
		map["hu"] = "hun";
		map["hy"] = "arm";
		map["hz"] = "her";
		map["ia"] = "ina";
		map["id"] = "ind";
		map["ie"] = "ile";
		map["ig"] = "ibo";
		map["ii"] = "iii";
		map["ik"] = "ipk";
		map["io"] = "ido";
		map["is"] = "ice";
		map["it"] = "ita";
		map["iu"] = "iku";
		map["ja"] = "jpn";
		map["jv"] = "jav";
		map["ka"] = "geo";
		map["kg"] = "kon";
		map["ki"] = "kik";
		map["kj"] = "kua";
		map["kk"] = "kaz";
		map["kl"] = "kal";
		map["km"] = "khm";
		map["kn"] = "kan";
		map["ko"] = "kor";
		map["kr"] = "kau";
		map["ks"] = "kas";
		map["ku"] = "kur";
		map["kv"] = "kom";
		map["kw"] = "cor";
		map["ky"] = "kir";
		map["la"] = "lat";
		map["lb"] = "ltz";
		map["lg"] = "lug";
		map["li"] = "lim";
		map["ln"] = "lin";
		map["lo"] = "lao";
		map["lt"] = "lit";
		map["lu"] = "lub";
		map["lv"] = "lav";
		map["mg"] = "mlg";
		map["mh"] = "mah";
		map["mi"] = "mao";
		map["mk"] = "mac";
		map["ml"] = "mal";
		map["mn"] = "mon";
		map["mr"] = "mar";
		map["ms"] = "may";
		map["mt"] = "mlt";
		map["my"] = "bur";
		map["na"] = "nau";
		map["nb"] = "nob";
		map["nd"] = "nde";
		map["ne"] = "nep";
		map["ng"] = "ndo";
		map["nl"] = "dut";
		map["nn"] = "nno";
		map["no"] = "nor";
		map["nr"] = "nbl";
		map["nv"] = "nav";
		map["ny"] = "nya";
		map["oc"] = "oci";
		map["oj"] = "oji";
		map["om"] = "orm";
		map["or"] = "ori";
		map["os"] = "oss";
		map["pa"] = "pan";
		map["pi"] = "pli";
		map["pl"] = "pol";
		map["ps"] = "pus";
		map["pt"] = "por";
		map["qu"] = "que";
		map["rm"] = "roh";
		map["rn"] = "run";
		map["ro"] = "rum";
		map["ru"] = "rus";
		map["rw"] = "kin";
		map["sa"] = "san";
		map["sc"] = "srd";
		map["sd"] = "snd";
		map["se"] = "sme";
		map["sg"] = "sag";
		map["si"] = "sin";
		map["sk"] = "slo";
		map["sl"] = "slv";
		map["sm"] = "smo";
		map["sn"] = "sna";
		map["so"] = "som";
		map["sq"] = "alb";
		map["sr"] = "srp";
		map["ss"] = "ssw";
		map["st"] = "sot";
		map["su"] = "sun";
		map["sv"] = "swe";
		map["sw"] = "swa";
		map["ta"] = "tam";
		map["te"] = "tel";
		map["tg"] = "tgk";
		map["th"] = "tha";
		map["ti"] = "tir";
		map["tk"] = "tuk";
		map["tl"] = "tgl";
		map["tn"] = "tsn";
		map["to"] = "ton";
		map["tr"] = "tur";
		map["ts"] = "tso";
		map["tt"] = "tat";
		map["tw"] = "twi";
		map["ty"] = "tah";
		map["ug"] = "uig";
		map["uk"] = "ukr";
		map["ur"] = "urd";
		map["uz"] = "uzb";
		map["ve"] = "ven";
		map["vi"] = "vie";
		map["vo"] = "vol";
		map["wa"] = "wln";
		map["wo"] = "wol";
		map["xh"] = "xho";
		map["yi"] = "yid";
		map["yo"] = "yor";
		map["za"] = "zha";
		map["zh"] = "chi";
		map["zu"] = "zul";
	}

	return map.contains(iso2Code) ? map[iso2Code] : QString();
}

QString
LanguageCode::nameFromIso2(const QString &iso2Code)
{
	QString name = KGlobal::locale()->languageCodeToName(iso2Code);
	return name.isEmpty() ? i18n("Unknown") : name;
}

QString
LanguageCode::nameFromIso3(const QString &iso3Code)
{
	QString name = KGlobal::locale()->languageCodeToName(toIso2(iso3Code));
	return name.isEmpty() ? i18n("Unknown") : name;
}

/*static*/ QString
LanguageCode::nameFromIso(const QString &isoCode)
{
	QString name = KGlobal::locale()->languageCodeToName(isoCode.length() == 2 ? isoCode : toIso2(isoCode));
	return name.isEmpty() ? i18n("Unknown") + " (" + isoCode + ")" : name;
}
