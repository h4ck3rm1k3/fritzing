/*
 * (c) Fachhochschule Potsdam
 */

#ifndef FRITZINGSVGPATH_H_
#define FRITZINGSVGPATH_H_

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

	QString absolutePath(const QString &sep = "/") {
		return first+sep+second+sep+third;
	}
};

#endif /* FRITZINGSVGPATH_H_ */
