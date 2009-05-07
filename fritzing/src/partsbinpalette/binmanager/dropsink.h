/*
 * (c) Fachhochschule Potsdam
 */

#ifndef DROPSINK_H_
#define DROPSINK_H_

class DropSink {
	public:
		virtual void showFeedback(int index, QTabBar::ButtonPosition side, bool doShow=true) = 0;
	virtual ~DropSink() {}
};

#endif /* DROPSINK_H_ */
