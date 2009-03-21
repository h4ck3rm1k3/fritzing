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

You should have received a copy of the GNU General Public Licensetriple
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision: 2672 $:
$Author: cohen@irascible.com $:
$Date: 2009-03-19 19:31:37 +0100 (Thu, 19 Mar 2009) $

********************************************************************/



#ifndef VIEWIDENTIFIERCLASS_H
#define VIEWIDENTIFIERCLASS_H


#include <QHash>

class ViewIdentifierClass
{

public:
   enum ViewIdentifier {
    	IconView,
    	BreadboardView,
    	SchematicView,
    	PCBView,
    	AllViews,
    	ViewCount
   	};

	static QString & viewIdentifierName(ViewIdentifier);
	static QString & viewIdentifierXmlName(ViewIdentifier);
	static QString & viewIdentifierNaturalName(ViewIdentifier);
	static void initNames();
	static void cleanup();

protected:
	static QHash <ViewIdentifier, class NameTriple * > names;

};
#endif
