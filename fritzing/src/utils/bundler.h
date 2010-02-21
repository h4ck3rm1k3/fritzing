

#ifndef BUNDLER_H_
#define BUNDLER_H_

class Bundler {
public:
	virtual ~Bundler() {}
	virtual bool saveAsAux(const QString &filename) = 0;
	virtual void loadBundledAux(QDir &dir, QList<class ModelPart*> mps) {Q_UNUSED(dir); Q_UNUSED(mps);};
};

#endif /* BUNDLER_H_ */
