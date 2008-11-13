#ifndef VIEWGEOMETRY_H
#define VIEWGEOMETRY_H


#include <QPointF>
#include <QDomElement>
#include <QLineF>
#include <QRectF>
#include <QTransform>

class ViewGeometry
{

public:
	ViewGeometry();
	ViewGeometry(QDomElement &);
	ViewGeometry(const ViewGeometry &);
	~ViewGeometry() {}

public:
	enum WireFlag {
		NoFlag = 0,
		VirtualFlag = 1,
		RoutedFlag = 2,
		TraceFlag = 4,
		JumperFlag = 8,
		RatsnestFlag = 16,
		AutoroutableFlag = 32,
	};
	Q_DECLARE_FLAGS(WireFlags, WireFlag)


protected:
	void setWireFlag(bool setting, WireFlag flag);

public:
	void setZ(qreal);
	qreal z() const;
	void setLoc(QPointF);
	QPointF loc() const;
	void setLine(QLineF);
	QLineF line() const;
	void offset(qreal x, qreal y);
	bool selected();
	void setSelected(bool);
	QRectF rect() const;
	void setTransform(QTransform);
	QTransform transform() const;
	void set(const ViewGeometry &);
	void setVirtual(bool);
	bool getVirtual() const;
	void setRouted(bool);
	bool getRouted() const;
	void setTrace(bool);
	bool getTrace() const;
	void setJumper(bool);
	bool getJumper() const;
	void setRatsnest(bool);
	bool getRatsnest() const;
	void setAutoroutable(bool);
	bool getAutoroutable() const;
	void setWireFlags(WireFlags);
	bool hasFlag(WireFlag);
	bool hasAnyFlag(WireFlags);
	int flagsAsInt() const;
	ViewGeometry::WireFlags wireFlags();

protected:
	qreal m_z;
	QPointF m_loc;
	QLineF m_line;
	QRectF m_rect;
	bool m_selected;
	WireFlags m_wireFlags;
	QTransform m_transform;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(ViewGeometry::WireFlags)

 //Q_DECLARE_METATYPE(ViewGeometry *);

#endif
