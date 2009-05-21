/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include <QHash>
#include <QVector>

#ifdef Q_WS_WIN
#ifndef QT_NO_DEBUG
// windows hack for finding memory leaks
// the 'new' redefinition breaks QHash and QVector so they are included beforehand.
#define _CRTDBG_MAP_ALLOC
#include <iostream>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

#ifdef Q_WS_WIN
#define getenvUser() getenv("USERNAME")
#else
#define getenvUser() getenv("USER")
#endif

#include <QString>
#include <QDir>
#include <QDomElement>
#include <QStringList>
#include <QPair>
#include <QList>

#define ALLMOUSEBUTTONS (Qt::LeftButton | Qt::MidButton | Qt::RightButton | Qt::XButton1 | Qt::XButton2)

typedef QPair<qreal, qreal> RealPair;
typedef QPair<QString, QString> StringPair;

QString getUserPartsFolder();
QDir *getApplicationSubFolder(QString);
QString getApplicationSubFolderPath(QString);

QDomElement findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue);

qreal convertToInches(const QString & string, bool * ok);

bool isParent(QObject * candidateParent, QObject * candidateChild);

static QString ___emptyString___;
static QDomElement ___emptyElement___;
static QStringList ___emptyStringList___;
static QHash<QString, QString> ___emptyStringHash___;
static QDir ___emptyDir___;
static QByteArray ___emptyByteArray___;

#ifdef Q_WS_MAC
static const QString ___MacStyle___ = " QTabBar::tab {margin-bottom: 10px; min-width: 15ex;} ";
#else
static const QString ___MacStyle___ = "";
#endif

static const QString FritzingSketchExtension(".fz");
static const QString FritzingBinExtension(".fzb");
static const QString FritzingPartExtension(".fzp");
static const QString FritzingBundledPartExtension(".fzpz");
static const QString FritzingModuleExtension(".fzm");
static const QString FritzingBundleExtension(".fzz");

const QList<QString> & fritzingExtensions();

static const QString QtFunkyPlaceholder("[*]");  // this is some wierd hack Qt uses in window titles as a placeholder to setr the modified state

#endif
