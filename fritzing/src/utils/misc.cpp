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

// misc Fritzing utility functions
#include "misc.h"
#include "../debugdialog.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QTextStream>

static QList<QString> ___fritzingExtensions___;

const QList<QString> & fritzingExtensions() {
	if (___fritzingExtensions___.count() == 0) {
		___fritzingExtensions___
			<< FritzingSketchExtension << FritzingBinExtension
			<< FritzingPartExtension << FritzingModuleExtension
			<< FritzingBundleExtension << FritzingBundledPartExtension
			<< FritzingBundledBinExtension;
	}

	return ___fritzingExtensions___;
}

// finds the user parts folder based on local desktop (OS) defaults
QString getUserPartsFolder(){
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/UserCreatedParts";
}

// finds a subfolder of the application directory searching backward up the tree
QDir * getApplicationSubFolder(QString search) {
	QString path = QCoreApplication::applicationDirPath();
    path += "/" + search;
	//DebugDialog::debug(QString("path %1").arg(path) );
    QDir* dir= new QDir(path);
    while (!dir->exists()) {
    	// if we're running from the debug or release folder, go up one to find things
    	dir->cdUp();
		dir->cdUp();
    	if (dir->isRoot()) return NULL;   // didn't find the search folder

		dir->setPath(dir->absolutePath() + "/" + search);
   	}

   	return dir;
}

QString getApplicationSubFolderPath(QString search) {
	QDir * dir = getApplicationSubFolder(search);
	if (dir == NULL) return "";

	QString result = dir->path();
	delete dir;
	return result;
}

QString getUserDataStorePath(QString folder) {
	QString settingsFile = QSettings(QSettings::IniFormat,QSettings::UserScope,"Fritzing","Fritzing").fileName();
	return QFileInfo(settingsFile).dir()
		.absolutePath()+(folder!=___emptyString___?"/"+folder:"");
}

QStringList getUserDataStoreFolders() {
	QStringList folders;
	folders << "/bins"
		<< "/parts/user" << "/parts/contrib"
		<< "/parts/svg/user/icon" << "/parts/svg/user/breadboard"
		<< "/parts/svg/user/schematic" << "/parts/svg/user/pcb"
		<< "/parts/svg/contrib/icon" << "/parts/svg/contrib/breadboard"
		<< "/parts/svg/contrib/schematic" << "/parts/svg/contrib/pcb";
	return folders;
}

qreal convertToInches(const QString & s, bool * ok) {
	QString string = s;
	qreal divisor = 1.0;
	if (string.endsWith("cm", Qt::CaseInsensitive)) {
		divisor = 2.54;
		string.chop(2);
	}
	else if (string.endsWith("mm", Qt::CaseInsensitive)) {
		divisor = 25.4;
		string.chop(2);
	}
	else if (string.endsWith("in", Qt::CaseInsensitive)) {
		divisor = 1.0;
		string.chop(2);
	}
	else {
		if (ok) *ok = false;
		return 0;
	}

	bool fine;
	qreal result = string.toDouble(&fine);
	if (!fine) {
		if (ok) *ok = false;
		return 0;
	}

	if (ok) *ok = true;
	return result / divisor;
}


bool isParent(QObject * candidateParent, QObject * candidateChild) {
	QObject * parent = candidateChild->parent();
	while (parent) {
		if (parent == candidateParent) return true;

		parent = parent->parent();
	}

	return false;
}

QDomElement findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue) {
	if (element.hasAttribute(attributeName)) {
		if (element.attribute(attributeName).compare(attributeValue) == 0) return element;
	}

     for(QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
     {
		 QDomElement result = findElementWithAttribute(e, attributeName, attributeValue);
		 if (!result.isNull()) return result;
     }

	return ___emptyElement___;
}

bool createFolderAnCdIntoIt(QDir &dir, QString newFolder) {
	if(!dir.mkdir(newFolder)) return false;
	if(!dir.cd(newFolder)) return false;

	return true;
}

void distanceFromLine(double cx, double cy, double ax, double ay, double bx, double by, 
					  double & dx, double & dy, double &distanceSegment, bool & atEndpoint)
{

	// http://www.codeguru.com/forum/showthread.php?t=194400

	//
	// find the distance from the point (cx,cy) to the line
	// determined by the points (ax,ay) and (bx,by)
	//

	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;
     
	if ( (r >= 0) && (r <= 1) )
	{
		dx = ax + r*(bx-ax);
		dy = ay + r*(by-ay);
		distanceSegment = (cx-dx)*(cx-dx) + (cy-dy)*(cy-dy);
		atEndpoint = false;
	}
	else
	{
		atEndpoint = true;
		double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
		double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
		if (dist1 < dist2)
		{
			dx = ax;
			dy = ay;
			distanceSegment = dist1;
		}
		else
		{
			dx = bx;
			dy = by;
			distanceSegment = dist2;
		}
	}

	return;
}

QPointF calcConstraint(Constraint& constraint, QPointF initial, QPointF current) {
	if (constraint == NO_CONSTRAINT) {
		qreal dx = qAbs(current.x() - initial.x());
		qreal dy = qAbs(current.y() - initial.y());

		if (dx <= 2 && dy <= 2) return initial;

		if (dy == 0) constraint = VERTICAL_CONSTRAINT;
		else if (dx == 0) constraint = HORIZONTAL_CONSTRAINT;
		else if ((dx / dy) < 2.0 && (dx / dy) > .5) constraint = FORTY_FIVE_CONSTRAINT;
		else if (dx > dy) constraint = HORIZONTAL_CONSTRAINT;
		else constraint = VERTICAL_CONSTRAINT;
	}

	QPointF result;
	if (constraint == VERTICAL_CONSTRAINT) {
		result.setY(initial.y());
		result.setX(current.x());
	}
	else if (constraint == HORIZONTAL_CONSTRAINT) {
		result.setX(initial.x());
		result.setY(current.y());
	}
	else {
		qreal dx = current.x() - initial.x();
		qreal ax = qAbs(dx);
		qreal dy = current.y() - initial.y();
		qreal ay = qAbs(dy);
		qreal d = qMin(ax, ay);					// qmax?
		if (dx == 0) {
			ax = dx = 1;
		}
		if (dy == 0) {
			ay = dy = 1;
		}
		result.setX(initial.x() + (d * ax / dx));
		result.setY(initial.y() + (d * ay / dy));
	}

	return result;
}

// this function searches by regexp
bool containsText(const QString &filepath, const QString &searchText) {
	QRegExp re(searchText);
	if(!re.isValid()) return false;

    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly )) return false;

	QTextStream stream(&file);
	QString content = stream.readAll();
	file.close();

	return re.indexIn(content) != -1;
}
