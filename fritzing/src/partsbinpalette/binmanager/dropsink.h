/*
 * (c) Fachhochschule Potsdam
 */

#ifndef DROPSINK_H_
#define DROPSINK_H_

class DropSink {
	public:
		virtual void showFeedback(int index, bool doShow=true) = 0;
	virtual ~DropSink() {}
};

#endif /* DROPSINK_H_ */
