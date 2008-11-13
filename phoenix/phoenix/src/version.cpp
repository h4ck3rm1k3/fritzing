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

#include "version.h"
						
#include <QString>
#include <QStringList>
				
QString Version::m_majorVersion("0");
QString Version::m_minorVersion("1");
QString Version::m_modifier("b");
QString Version::m_svnRevision("$Revision$:");
QString Version::m_revision;
QString Version::m_versionString;
Version * Version::m_singleton = new Version();
											
Version::Version() {
	m_revision = "";
	QStringList strings = m_svnRevision.split(" ", QString::SkipEmptyParts);
	if (strings.size() >= 2) {
		m_revision = strings[1];
	}

	m_versionString = QString("%1.%2.%3.%4").arg(m_majorVersion).arg(m_minorVersion).arg(m_modifier).arg(m_revision);
}

const QString & Version::majorVersion() {
	return m_majorVersion;
}

const QString & Version::minorVersion() {
	return m_minorVersion;
}

const QString & Version::revision() {
	return m_revision;
}

const QString & Version::modifier() {
	return m_modifier;
}

const QString & Version::versionString() {
	return m_versionString;
}

