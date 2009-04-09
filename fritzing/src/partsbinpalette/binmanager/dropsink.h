/*
 * (c) Fachhochschule Potsdam
 */

#ifndef DROPSINK_H_
#define DROPSINK_H_

class DropSink {
	public:
		virtual void showFeedback(int index, bool doShow=true) = 0;
};

#endif /* DROPSINK_H_ */
