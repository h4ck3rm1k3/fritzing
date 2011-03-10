/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef PROPERTYDEF_H
#define PROPERTYDEF_H

#include <QHash>
#include <QList>

struct PropertyDef {
	QString name;
	QString id;
	QString symbol;
	qreal minValue;
	qreal maxValue;
	qreal defaultValue;
	QList<qreal> menuItems;
};

struct InstanceDef {
	QString moduleID;
	QList<PropertyDef *> propertyDefs;
};

class PropertyDefMaster
{
public:
	static void initPropertyDefs(class ModelPart *, QHash<PropertyDef *, QString> & propertyDefs);

protected:
	static void loadPropertyDefs();

protected:
	static QHash <QString, PropertyDef *> PropertyDefs;
	static QHash <QString, InstanceDef *> InstanceDefs;
};

#endif // PROPERTYDEF_H
