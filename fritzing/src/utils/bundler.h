

#ifndef BUNDLER_H_
#define BUNDLER_H_

class Bundler {
public:
	virtual void saveAsAux(const QString &filename) = 0;
	virtual void loadBundledAux(QDir &dir) {Q_UNUSED(dir)};
};

#endif /* BUNDLER_H_ */
