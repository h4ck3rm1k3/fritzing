#ifndef MISC_H
#define MISC_H

#include <QPair>
#include <QString>
#include <QDir>
#include <QDomElement>
#include <QStringList>
#include <QHash>

template <class T1, class T2, class T3>
struct Triple {
public:
	Triple() {}

	Triple(T1 _first, T2 _second, T3 _third) {
		first = _first;
		second = _second;
		third = _third;
	}

	T1 first;
	T2 second;
	T3 third;
};

typedef QPair<qreal, qreal> RealPair;
typedef QPair<QString, QString> StringPair;
typedef Triple<QString, QString, QString> StringTriple;

QString getUserPartsFolder();
QDir *getApplicationSubFolder(QString);
QString getApplicationSubFolderPath(QString);

static QString ___emptyString___;
static QDomElement ___emptyElement___;
static QStringList ___emptyStringList___;
static QHash<QString, QString> ___emptyStringHash___;
static QDir ___emptyDir___;

#ifdef Q_WS_MAC
static const QString ___MacStyle___ = " QTabBar::tab {margin-bottom: 10px; min-width: 15ex;} ";
#else
static const QString ___MacStyle___ = "";
#endif

#endif
