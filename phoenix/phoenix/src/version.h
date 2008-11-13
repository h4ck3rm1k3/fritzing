// -------------------------------------------
// (c) 2008 Fachhochschule Potsdam
//
// This file is open source, part of the Friting project www.fritzing.org.
// You may use this code as you like
// as long as you use this header information.
//
// FHP is not responsible for any of the contents or what use you make of it.
//
//--------------------------------------------
//
// $Revision$:
// $Author$:
// $Date$
//
//--------------------------------------------

#ifndef VERSION_H
#define VERSION_H

#include <QString>

class Version {

public:
	static const QString & majorVersion();
	static const QString & minorVersion();
	static const QString & modifier();
	static const QString & revision();
	static const QString & versionString();

protected:
	Version();

protected:

	static QString m_majorVersion;
	static QString m_minorVersion;
	static QString m_svnRevision;
	static QString m_revision;
	static QString m_modifier;
	static QString m_versionString;
	static Version * m_singleton;

};

#endif
