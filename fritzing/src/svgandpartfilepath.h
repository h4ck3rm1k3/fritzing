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

#ifndef SVGANDPARTFILEPATH_H_
#define SVGANDPARTFILEPATH_H_

#include "misc.h"


/*
 * While the svg and parts files, are loaded dynamically from different folders
 * (core, contrib and user), this structure let us keep truck of this structure,
 * without the need to deal with substrings creation.
 * So, for example, if an svg file is in "/fritzing/parts/svg/core/breadboard/svg_file.svg" then
 * partFolderPath == "/fritzing/parts/svg"
 * coreContribOrUser == "core"
 * fileRelativePath == "breadboard/svg_file.svg"
 */

class SvgAndPartFilePath : public StringTriple {
public:
	SvgAndPartFilePath() : StringTriple() {}
	SvgAndPartFilePath(QString partsPath, QString folderInParts, QString relativeFilePath)
		: StringTriple(partsPath, folderInParts, relativeFilePath) {}

	const QString &partFolderPath() {
		return first;
	}
	void setPartFolderPath(const QString &partFolderPath) {
		first = partFolderPath;
	}

	const QString &coreContribOrUser() {
		return second;
	}
	void setCoreContribOrUser(const QString &coreContribOrUser) {
		second = coreContribOrUser;
	}

	const QString &fileRelativePath() {
		return third;
	}
	void setFileRelativePath(const QString &fileRelativePath) {
		third = fileRelativePath;
	}

	QString absolutePath() {
		QString sep = "/";
		return first+sep+second+sep+third;
	}
};

#endif /* SVGANDPARTFILEPATH_H_ */
