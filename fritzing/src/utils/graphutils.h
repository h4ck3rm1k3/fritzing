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

$Revision: 4228 $:
$Author: cohen@irascible.com $:
$Date: 2010-06-09 00:55:55 +0200 (Wed, 09 Jun 2010) $

********************************************************************/

#ifndef GRAPHUTILS_H
#define GRAPHUTILS_H

#include "../connectors/connectorItem.h"

class GraphUtils
{

public:
	static bool chooseRatsnestGraph(const QList<ConnectorItem *> & equipotentials, ConnectorPairHash & result);

};

#endif
