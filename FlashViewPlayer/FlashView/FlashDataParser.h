#ifndef __FLASH_VIEW__
#define __FLASH_VIEW__

#include <iostream>
#include <unordered_map>
#include <vector>
#include <QPainter>
#include <QImage>
#include <QThread>
#include <QMutex>
#include "json.h"
#include "AnimData.h"
#include "KeyFrameData.h"
#include "FlashViewCallback.h"

namespace shine {

class vFlashThread : public QThread
{
	Q_OBJECT
public:
	vFlashThread(QObject *parent = Q_NULLPTR);
	~vFlashThread();
	void addFlashItem(const QString &name, const QString &path);
	void startFlashThread();
	void stopFlashThread();
	const QPixmap getImage(const QString& name);

protected:
	virtual void run();

private:
	bool m_start = false;
	QMutex m_mutex;
	QVector<QPair<QString, QString>> m_items;
	QHash<QString, QPixmap> m_images;
};
	

class FlashDataParser {
	//线程中加载动画图片
	vFlashThread m_ThreadLoadImage;
	std::unordered_map<std::string, AnimData*> mParsedData;
	int mFrameRate = -1;
	unsigned int mParseFrameMaxIndex = 0;
	std::vector<std::string> mFlashName;
	std::string mFlashDir;
	double mOneFrameTime = -1;
	bool mIsInitOk;
	bool mIsPause = true;
	bool mIsStop = true;
	double mTotalTime;
	std::vector<std::string> mRunningAnimName;
	int mSetLoopTimes;
	int mLoopTimes;
	int mFromIndex;
	int mToIndex;
	int mLastFrameIndex = -1;
	int mStopWithIndex = -1;
	float *mExtraScaleX = nullptr;
	float *mExtraScaleY = nullptr;
	float mScale = 1;
	QMatrix mMatrix;
	FlashViewCallback mFlashViewEventCallback;
	//int mDesignResolution = 750;

public:
	static const int FlashLoopTimeOnce = 1;
	static const int FlashLoopTimeForever = 0;
protected:
	//void setupScale(const QPainter & painter);
	void clearData();
	bool init();

private:
	void parseAnims(const Json::Value& anims);
	void parseKeyFrame(const Json::Value& oneFrame, AnimData* parsedAnim, const Json::Value& nextFrame);
	float calcPercentValue(float oldValue, float newValue, bool isSkewXY, float per);
	void addOneFrameDataToIdx(const Json::Value& oneFrame, int idx, int first, AnimData* parsedAnim);
	void addOneFrameDataToIdx(KeyFrameData* frameData, int idx, AnimData* parsedAnim);
	std::vector<KeyFrameData *>* createArrForIdx(int idx, AnimData* parsedAnim);
	void parseJson(const Json::Value& jsonData, const std::string &root);
	void drawImage(QPainter &painter, KeyFrameData *frameData);
	void checkMark(std::string animName, int frameIndex);

public:
	FlashDataParser();
	~FlashDataParser();
	bool reload(const std::vector<std::string> &flashName, const std::string &flashDir, float scale = 1.f);
	bool play(const std::vector<std::string> & animName, int loopTimes, int fromIndex, int toIndex);
	void stop();
	void pause();
	bool isInitOk();
	unsigned int getParseFrameMaxIndex();
	std::vector<std::string> getRunningAnimName();
	double getOneFrameTime();
	void increaseTotalTime(double increaseValue);
	bool draw(QPainter &painter);
	void draw(QPainter &painter, int frameIndex,const std::vector<std::string> &animName, bool isTriggerEvent);
	void setEventCallback(const FlashViewCallback &callback);
	void handleStart();
	bool isPlaying();
	bool isPaused();
	bool isStoped();
	/*
	*
	* 最后停到哪个界面
	* 0是第一帧
	* -1是最后一帧
	*
	* */
	void stopWithIndex(int index);
	void stopAtLastIndex(QPainter &painter);
	/*
	*额外缩放
	*/
	void setExtraScale(float* x, float* y);
	/*
	*根据帧获取当前图片
	*/
	QString getImageFileName(int index);
};

}

#endif
