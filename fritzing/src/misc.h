/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef MISC_H
#define MISC_H

#include <QPair>
#include <QString>
#include <QDir>
#include <QDomElement>
#include <QStringList>
#include <QHash>

template <class T1, class T2, class T3>
struct Triple {
public:
	Triple() {}

	Triple(T1 _first, T2 _second, T3 _third) {
		first = _first;
		second = _second;
		third = _third;
	}

	T1 first;
	T2 second;
	T3 third;
};

typedef QPair<qreal, qreal> RealPair;
typedef QPair<QString, QString> StringPair;
struct StringTriple : public Triple<QString, QString, QString> {
	StringTriple() : Triple<QString, QString, QString>() {}
	StringTriple(QString _first, QString _second, QString _third)
		: Triple<QString, QString, QString>(_first, _second, _third) {}
};

QString getUserPartsFolder();
QDir *getApplicationSubFolder(QString);
QString getApplicationSubFolderPath(QString);

qreal convertToInches(const QString & string, bool * ok);

bool isParent(QObject * candidateParent, QObject * candidateChild);

static QString ___emptyString___;
static QDomElement ___emptyElement___;
static QStringList ___emptyStringList___;
static QHash<QString, QString> ___emptyStringHash___;
static QDir ___emptyDir___;

static QString ITEMBASE_FONT_PREFIX = "<font size='2'>";
static QString ITEMBASE_FONT_SUFFIX = "</font>";

static const int STANDARD_ICON_IMG_WIDTH = 32;
static const int STANDARD_ICON_IMG_HEIGHT = 32;

static const qreal ROUTED_OPACITY = 0.35;
static const qreal UNROUTED_OPACITY = 1.0;

#ifdef Q_WS_MAC
static const QString ___MacStyle___ = " QTabBar::tab {margin-bottom: 10px; min-width: 15ex;} ";
#else
static const QString ___MacStyle___ = "";
#endif

#endif
