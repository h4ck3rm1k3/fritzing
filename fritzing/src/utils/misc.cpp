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
#include <QSet>

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
	else if (string.endsWith("px", Qt::CaseInsensitive)) {
		divisor = 72.0;
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

struct PD {
	QPointF p;
	qreal d;
};

bool pdLessThan(PD* pd1, PD* pd2) {
	return pd1->d < pd2->d;
}

QPointF calcConstraint(QPointF initial, QPointF current) {
	QList<PD *> pds;

	PD * pd = new PD;
	pd->p.setX(current.x());
	pd->p.setY(initial.y());
	pd->d = (current.y() - initial.y()) * (current.y() - initial.y());
	pds.append(pd);

	pd = new PD;
	pd->p.setX(initial.x());
	pd->p.setY(current.y());
	pd->d = (current.x() - initial.x()) * (current.x() - initial.x());
	pds.append(pd);

	qreal dx, dy, d;
	bool atEndpoint;

	QLineF plus45(initial.x() - 10000, initial.y() - 10000, initial.x() + 10000, initial.y() + 10000);
	distanceFromLine(current.x(), current.y(), plus45.p1().x(), plus45.p1().y(), plus45.p2().x(), plus45.p2().y(), dx, dy, d, atEndpoint);
	pd = new PD;
	pd->p.setX(dx);
	pd->p.setY(dy);
	pd->d = d;
	pds.append(pd);
		
	QLineF minus45(initial.x() + 10000, initial.y() - 10000, initial.x() - 10000, initial.y() + 10000);
	distanceFromLine(current.x(), current.y(), minus45.p1().x(), minus45.p1().y(), minus45.p2().x(), minus45.p2().y(), dx, dy, d, atEndpoint);
	pd = new PD;
	pd->p.setX(dx);
	pd->p.setY(dy);
	pd->d = d;
	pds.append(pd);

	qSort(pds.begin(), pds.end(), pdLessThan);
	QPointF result = pds[0]->p;
	foreach (PD* pd, pds) {
		delete pd;
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

QSet<QString> getRegexpCaptures(const QString &pattern, const QString &textToSearchIn) {
	QRegExp re(pattern);
	QSet<QString> captures;
	int pos = 0;

	while ((pos = re.indexIn(textToSearchIn, pos)) != -1) {
		captures << re.cap(1);
		pos += re.matchedLength();
	}

	return captures;
}
