#include "FlashDataParser.h"
#include <fstream>
#include <QtDebug>
#include <math.h>
#include <QMutexLocker>

void shine::FlashDataParser::clearData()
{
	//停止线程
	m_ThreadLoadImage.stopFlashThread();
}

bool shine::FlashDataParser::init()
{
	mIsInitOk = false;
	if (mFlashName.size() == 0 || mFlashDir.empty()) {
		//qDebug("[ERROR] mFlashName/mFlashDir is null, mFlashName:" + mFlashName.size());
		return false;
	}
	for (auto& data : mParsedData) {
		delete data.second;
	}
	mParsedData.clear();
	mParseFrameMaxIndex = 0;
	Json::Value jsonData;
	Json::Reader reader;
	for (int i = 0, len = mFlashName.size(); i < len; i++) {
		std::string jsonFileName = mFlashDir + "/" + mFlashName[i] + ".flajson";
		std::ifstream in(jsonFileName, std::ios::binary);
		if (!in.is_open()) {
			return false;
		}
		if (!reader.parse(in, jsonData)) {
			return false;
		}
		parseJson(jsonData, mFlashDir + "/" + mFlashName[i]);
	}
	mIsPause = false;
	//mDPIRate = 1.0f * widthPix / mDesignWidthResolution;

	//setScale(1, 1, true);

	mIsInitOk = true;
	return true;
}

//void shine::FlashDataParser::setupScale(const QPainter & painter) {
//	QRect rect = painter.viewport();
//	mScale = rect.width() > mDesignResolution ? 1: rect.width()/ (float)mDesignResolution;
//}

void shine::FlashDataParser::parseAnims(const Json::Value& anims)
{
	for (Json::ArrayIndex i = 0, c = anims.size(); i < c; i++) {
		AnimData* parsedAnim = new AnimData;
		auto animData = anims[i];
		std::string animName = animData["animName"].asString();
		int frameCount = animData["frameMaxNum"].asUInt();
		if (frameCount - 1 > mParseFrameMaxIndex) {
			mParseFrameMaxIndex = frameCount - 1;
		}
		parsedAnim->animFrameLen = frameCount;

		auto layers = animData["layers"];
		for (Json::ArrayIndex j = 0, count = layers.size(); j < count; j++) {
			auto oneLayer = layers[j];
			auto frames = oneLayer["frames"];
			Json::Value nextFrame;
			for (int l = 0, len = frames.size(); l < len; l++) {
				Json::Value oneFrame;
				oneFrame = nextFrame.isNull()? frames[l] : nextFrame;
				if (l < len - 1) {
					nextFrame = frames[l + 1];
				}
				parseKeyFrame(oneFrame, parsedAnim, nextFrame);
			}
		}
		mParsedData[animName] = parsedAnim;


	}
}

void shine::FlashDataParser::parseKeyFrame(const Json::Value & oneFrame, AnimData* parsedAnim, const Json::Value & nextFrame)
{
	bool isEmpty = oneFrame["isEmpty"].asBool();
	if (isEmpty) {
		return;
	}
	int duration = oneFrame["duration"].asInt();
	bool isTween = oneFrame["isTween"].asBool();
	int fromIdx = oneFrame["frameIndex"].asInt();
	int toIdx = fromIdx + duration;

	for (int i = fromIdx; i < toIdx; i++) {
		if (!isTween) {
			addOneFrameDataToIdx(oneFrame, i, fromIdx, parsedAnim);
		}
		else {
			float per = (i - fromIdx) / (float)duration;
			std::string texName = oneFrame["texName"].asString();
			float x = calcPercentValue(oneFrame["x"].asFloat(), nextFrame["x"].asFloat(), false, per);
			float y = calcPercentValue(oneFrame["y"].asFloat(), nextFrame["y"].asFloat(), false, per);
			float scaleX = calcPercentValue(oneFrame["scaleX"].asFloat(), nextFrame["scaleX"].asFloat(), false, per);
			float scaleY = calcPercentValue(oneFrame["scaleY"].asFloat(), nextFrame["scaleY"].asFloat(), false, per);
			float skewX = calcPercentValue(oneFrame.get("skewX", Json::Value(1)).asFloat(), nextFrame.get("skewX", Json::Value(1)).asFloat(), true, per);
			float skewY = calcPercentValue(oneFrame.get("skewY", Json::Value(1)).asFloat(), nextFrame.get("skewY", Json::Value(1)).asFloat(), true, per);
			float alpha = calcPercentValue(oneFrame.get("alpha", Json::Value(255)).asFloat(), nextFrame.get("alpha", Json::Value(255)).asFloat(), false, per);
			int ar = 0, ag = 0, ab = 0, aa = 0, br = 0, bg = 0, bb = 0, ba = 0;
			auto oneFrameColor = oneFrame["color"];
			auto nextFrameColor = nextFrame["color"];
			if (!oneFrameColor.isNull()) {
				ar = oneFrameColor["r"].asInt();
				ag = oneFrameColor["g"].asInt();;
				ab = oneFrameColor["b"].asInt();
				aa = oneFrameColor["a"].asInt();
			}
			if (!nextFrameColor.isNull()) {
				br = nextFrameColor["r"].asInt();
				bg = nextFrameColor["g"].asInt();
				bb = nextFrameColor["b"].asInt();
				ba = nextFrameColor["a"].asInt();
			}
			int r = (int)calcPercentValue(ar, br, false, per);
			int g = (int)calcPercentValue(ag, bg, false, per);
			int b = (int)calcPercentValue(ab, bb, false, per);
			int a = (int)calcPercentValue(aa, ba, false, per);
			std::string mark = oneFrame["mark"].asString();
			KeyFrameData *frameData = new KeyFrameData();
			frameData->texName = texName;
			frameData->x = x;
			frameData->y = y;
			frameData->scaleX = scaleX;
			frameData->scaleY = scaleY;
			frameData->skewX = skewX;
			frameData->skewY = skewY;
			frameData->alpha = alpha / 255.0f;
			frameData->blendColor = QColor(r, g, b, a);
			if (i == fromIdx && !mark.empty()) {
				frameData->mark = mark;
			}
			addOneFrameDataToIdx(frameData, i, parsedAnim);
		}
	}
}

float shine::FlashDataParser::calcPercentValue(float oldValue, float newValue, bool isSkewXY, float per)
{
	float ret = 0;
	float span = fabsf(newValue - oldValue);
	if (span > 180 && isSkewXY) {
		float realSpan = 360 - span;
		float mark = (oldValue < 0) ? -1 : 1;
		float mid = 180 * mark;
		float newStart = -mid;
		float midPer = fabsf(mid - oldValue) / realSpan;
		if (per < midPer) {
			ret = oldValue + per * realSpan * mark;
		}
		else {
			ret = newStart + (per - midPer) * realSpan * mark;
		}
	}
	else {
		ret = oldValue + per * (newValue - oldValue);
	}
	return ret;
}

void shine::FlashDataParser::addOneFrameDataToIdx(const Json::Value & oneFrame, int idx, int first, AnimData * parsedAnim)
{
	std::vector<KeyFrameData *>* arr = createArrForIdx(idx, parsedAnim);
	if (!oneFrame.isNull()) {
		arr->push_back(new KeyFrameData(oneFrame, idx == first));
	}

}

void shine::FlashDataParser::addOneFrameDataToIdx(KeyFrameData * frameData, int idx, AnimData * parsedAnim)
{
	std::vector<KeyFrameData *>* arr = createArrForIdx(idx, parsedAnim);
	arr->push_back(frameData);
}

std::vector<shine::KeyFrameData*>* shine::FlashDataParser::createArrForIdx(int idx, AnimData * parsedAnim)
{
	std::string sIdx = std::to_string(idx);
	if (parsedAnim->keyFrameData == nullptr) {
		parsedAnim->keyFrameData = new std::unordered_map<std::string, std::vector<KeyFrameData*>*>;
	}
	std::vector<KeyFrameData *>* arr = nullptr;
	if (parsedAnim->keyFrameData->find(sIdx) == parsedAnim->keyFrameData->end()) {
		arr = new std::vector<KeyFrameData*>;
		(*parsedAnim->keyFrameData)[sIdx] = arr;
	}
	else {
		arr = (*parsedAnim->keyFrameData)[sIdx];
	}
	return arr;
}

void shine::FlashDataParser::parseJson(const Json::Value & jsonData, const std::string & root)
{
	//读取帧率
	mFrameRate = jsonData["frameRate"].asInt();
	mOneFrameTime = 1.0 / mFrameRate;
	//读取textures名字
	auto textures = jsonData["textures"];
	std::vector<std::string> texNames;
	//线程不安全的
	for (Json::ArrayIndex i = 0, c = textures.size(); i < c; i++) {
		auto textName = textures[i].asString();
		auto path = QString::fromStdString(root + "/" + textName);
		//线程中加载资源
		m_ThreadLoadImage.addFlashItem(QString::fromStdString(textName), path);
	}
	m_ThreadLoadImage.startFlashThread();
	auto anims = jsonData["anims"];
	parseAnims(anims);
}


shine::FlashDataParser::FlashDataParser()
{
}

shine::FlashDataParser::~FlashDataParser()
{
	clearData();
	for (auto& data : mParsedData) {
		delete data.second;
	}
	if (mExtraScaleX != nullptr) {
		delete[] mExtraScaleX;
		delete[] mExtraScaleY;
	}
}

bool shine::FlashDataParser::reload(const std::vector<std::string> &flashName, const std::string &flashDir, float scale)
{
	stop();
	if (scale > FLT_EPSILON) {
		mScale = scale;
	}
	else {
		mScale = 0.5;
	}
	if (mIsInitOk && mFlashDir == flashDir) {
		bool found = false;
		for (int i = 0, c = flashName.size(); i < c; i++) {
			found = false;
			for (int j = 0, l = mFlashName.size(); j < l; j++) {
				if (flashName[i] == mFlashName[j]) {
					found = true;
					break;
				}
			}
			if (!found) {
				break;
			}
		}
		if (found) {
			qDebug() << "FlashView-reload same";
			return true;
		}
	}
	clearData();
	mFlashName = flashName;
	mFlashDir = flashDir;
	//mDesignResolution = designResolution;
	return init();
}

bool shine::FlashDataParser::play(const std::vector<std::string>& animName, int loopTimes, int fromIndex, int toIndex)
{
	if (!mIsInitOk) {
		qDebug("[ERROR] call play when init error");
		return false;
	}
	for (int i = 0, len = animName.size(); i < len; i++) {
		std::string anim = animName[i];
		if (mParsedData.find(anim) == mParsedData.end()) {
			qDebug() << "[ERROR] play() cant find the animName " + QString::fromStdString(anim);
			return false;
		}
	}
	mIsStop = false;
	mIsPause = false;
	mTotalTime = 0;
	mRunningAnimName = animName;
	mSetLoopTimes = loopTimes;
	mLoopTimes = 0;

	mFromIndex = fromIndex;
	mToIndex = toIndex;
	return true;
}

void shine::FlashDataParser::stop()
{
	mTotalTime = 0;
	mRunningAnimName.clear();
	mSetLoopTimes = FlashLoopTimeOnce;
	mLoopTimes = 0;
	mIsStop = true;
	mLastFrameIndex = -1;
}

void shine::FlashDataParser::pause()
{
	if (!mIsInitOk) {
		qDebug("[ERROR] call resume when init error");
		return;
	}
	mIsPause = true;
}

bool shine::FlashDataParser::isInitOk()
{
	return mIsInitOk;
}

unsigned int shine::FlashDataParser::getParseFrameMaxIndex()
{
	return mIsInitOk? mParseFrameMaxIndex : 0;
}

std::vector<std::string> shine::FlashDataParser::getRunningAnimName()
{
	return mRunningAnimName;
}

double shine::FlashDataParser::getOneFrameTime()
{
	return mIsInitOk ? mOneFrameTime : 0;
}

void shine::FlashDataParser::increaseTotalTime(double increaseValue)
{
	if (!mIsInitOk) {
		qDebug("[ERROR] call increaseTotalTime when init error");
		return;
	}
	mTotalTime += increaseValue;
}

bool shine::FlashDataParser::draw(QPainter &painter)
{
	if (!mIsInitOk || mRunningAnimName.size() == 0 || mIsPause || mIsStop) {
		return false;
	}
	int animLen = mToIndex - mFromIndex;
	int currFrameIndex = mFromIndex + (int)(mTotalTime / mOneFrameTime + 0.5f) % animLen;
	//检查漏下的帧事件
	int mid = -1;
	if (mLastFrameIndex > currFrameIndex) {
		mid = mParseFrameMaxIndex;
	}
	for (int j = 0, len = mRunningAnimName.size(); j < len; j++) {
		std::string animName = mRunningAnimName[j];
		if (mid != -1) {
			for (int i = mLastFrameIndex + 1; i <= mid; i++) {
				checkMark(animName, i);
			}
			for (int i = 0; i < currFrameIndex; i++) {
				checkMark(animName, i);
			}
		}
		else {
			for (int i = mLastFrameIndex + 1; i < currFrameIndex; i++) {
				checkMark(animName, i);
			}
		}
	}
	draw(painter, currFrameIndex, mRunningAnimName, true);
	return true;

}

void shine::FlashDataParser::checkMark(std::string animName, int frameIndex)
{
	AnimData* animData = mParsedData[animName];
	if (animData == nullptr) {
		return;
	}
	std::vector<KeyFrameData *>* frameArr = (*animData->keyFrameData)[std::to_string(frameIndex)];
	if (frameArr == nullptr) {
		return;
	}
	for (int i = frameArr->size() - 1; i >= 0; i--) {
		KeyFrameData *frameData = (*frameArr)[i];
		if (!frameData->mark.empty() && nullptr != mFlashViewEventCallback) {
			FlashViewEventData *eventData = new FlashViewEventData();
			eventData->index = frameIndex;
			eventData->mark = frameData->mark;
			eventData->data = frameData->deepClone();
			mFlashViewEventCallback(FRAME, eventData);
			delete eventData;
		}
	}
}

void shine::FlashDataParser::draw(QPainter & painter, int frameIndex,const std::vector<std::string> & animName, bool isTriggerEvent)
{
	if (!mIsInitOk) {
		qDebug("[ERROR] call drawCanvas when init error");
		return;
	}
	//this->setupScale(painter);
	//检查是否到达最后一帧
	bool checkEnd = isTriggerEvent && mLastFrameIndex > frameIndex;
	if (checkEnd) {
		if (mSetLoopTimes != FlashLoopTimeForever) {
			if (mLoopTimes + 1 >= mSetLoopTimes) {
				if (mStopWithIndex == -1) {
					frameIndex = mToIndex;
				}
				else {
					frameIndex = mStopWithIndex;
				}

			}
		}
	}
	QRect rect = painter.viewport();
	for (int j = 0, len = animName.size(); j < len; j++) {
		painter.save();
		if (mExtraScaleX != nullptr && sizeof(*mExtraScaleX) / 4 > j) {
			float px = rect.width() / 2.0f, py = rect.height() / 2.0f;
			painter.translate(px,py);
			painter.scale(mExtraScaleX[j], mExtraScaleY[j]);
			painter.translate(- px, - py);
		}

		std::string anim = animName[j];
		AnimData *animData = mParsedData[anim];
		// 性能问题，用处不大，取消帧事件
		       /* if(isTriggerEvent && mFlashViewEventCallback != nullptr){
		            FlashViewEventData *eventData = new FlashViewEventData();
		            eventData->index = frameIndex;
					(*mFlashViewEventCallback)(FRAME, eventData);
					delete eventData;
		        }*/
		std::vector<KeyFrameData *> *frameArr = (*(animData->keyFrameData))[std::to_string(frameIndex)];


		if (frameArr != nullptr) {

			for (int i = frameArr->size() - 1; i >= 0; i--) {
				KeyFrameData *frameData = (*frameArr)[i];
				drawImage(painter, frameData);
				if (isTriggerEvent) {
					if (!frameData->mark.empty() && nullptr != mFlashViewEventCallback) {
						FlashViewEventData *eventData = new FlashViewEventData();
						eventData->index = frameIndex;
						eventData->mark = frameData->mark;
						eventData->data = frameData->deepClone();
						mFlashViewEventCallback(FRAME, eventData);
						delete eventData;
					}
				}
			}
		}
		else {
			qDebug() << QString::fromStdString(anim + " Blank Frame：" + std::to_string(frameIndex) + "，Frame Count：" + std::to_string(animData->animFrameLen));
		}
		painter.restore();
	}


	if (isTriggerEvent) {
		mLastFrameIndex = frameIndex;
		if (checkEnd) {
			if (mFlashViewEventCallback != nullptr) {
				mFlashViewEventCallback(ONELOOPEND, nullptr);
			}

			if (mSetLoopTimes >= FlashLoopTimeOnce) {
				if (++mLoopTimes >= mSetLoopTimes) {
					pause();
					if (mFlashViewEventCallback != nullptr) {
						mFlashViewEventCallback(STOP, nullptr);
					}
					
				}
			}
		}
		
	}
}

void shine::FlashDataParser::setEventCallback(const FlashViewCallback &callback)
{
	/*if (mFlashViewEventCallback != nullptr) {
		delete mFlashViewEventCallback;
	}*/
	mFlashViewEventCallback = std::move(callback);
}

void shine::FlashDataParser::handleStart()
{
	if (mFlashViewEventCallback != nullptr) {
		mFlashViewEventCallback(START, nullptr);
	}
}

bool shine::FlashDataParser::isPlaying()
{
	return !mIsStop && !mIsPause && mIsInitOk;
}

bool shine::FlashDataParser::isPaused()
{
	return mIsPause || !mIsInitOk;
}

bool shine::FlashDataParser::isStoped()
{
	return mIsStop || !mIsInitOk;
}

void shine::FlashDataParser::stopWithIndex(int index)
{
	mStopWithIndex = index;
}

void shine::FlashDataParser::stopAtLastIndex(QPainter & painter)
{
	draw(painter, mLastFrameIndex, mRunningAnimName, true);
}

void shine::FlashDataParser::setExtraScale(float * x, float * y)
{
	if (mExtraScaleX != nullptr) {
		delete[] mExtraScaleX;
		delete[] mExtraScaleY;
	}
	mExtraScaleX = x;
	mExtraScaleY = y;
}

QString shine::FlashDataParser::getImageFileName(int index)
{
	QString fileName = QString();
	for (int j = 0, len = mRunningAnimName.size(); j < len; j++) {
		std::string anim = mRunningAnimName[j];
		AnimData *animData = mParsedData[anim];
		if (nullptr == animData) {
			return fileName;
		}
		std::vector<KeyFrameData *> *frameArr = (*(animData->keyFrameData))[std::to_string(index)];
		if (nullptr != frameArr) {
			for (int i = frameArr->size() - 1; i >= 0; i--) {
				KeyFrameData *frameData = (*frameArr)[i];
				fileName = QString::fromStdString(frameData->texName);
			}
		}
	}
	return fileName;
}


void shine::FlashDataParser::drawImage(QPainter & painter, KeyFrameData * frameData)
{
	const auto img = m_ThreadLoadImage.getImage(QString::fromStdString(frameData->texName));
	if (img.isNull()) {
		return;
	}
	float imgW = img.width() * mScale;
	float imgH = img.height() * mScale;
	QRect rect = painter.viewport();
	QRect rect1 = painter.window();
	painter.save();
	float anchorX = frameData->x * mScale + rect.width() / 2.0f;
	float anchorY = -frameData->y * mScale + rect.height() / 2.0f;
	painter.translate(anchorX, anchorY);

	mMatrix.reset();//[1 0 0 1 0 0]
	if (frameData->skewX == frameData->skewY) {
		mMatrix.rotate(frameData->skewX);
	}
	else {
		float radiusX = 0.01745329252f * frameData->skewX;
		float radiusY = 0.01745329252f * frameData->skewY;
		float cx = cosf(radiusX);
		float sx = sinf(radiusX);
		float cy = cosf(radiusY);
		float sy = sinf(radiusY);

		qreal a = cy * mMatrix.m11();
		qreal b = sy * mMatrix.m11();
		qreal c = - sx * mMatrix.m22();
		qreal d = cx * mMatrix.m22();

		mMatrix.setMatrix(a, b, c, d, 0, 0);
	}

	painter.setMatrix(mMatrix,true);

	//缩放
	painter.scale(frameData->scaleX, frameData->scaleY);

	painter.translate(-0.5f * imgW, -0.5f * imgH);
	painter.setOpacity(frameData->alpha);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawPixmap(QRect(0, 0, imgW, imgH), img);
	if (frameData->blendColor.value() != 0) {
		painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
		painter.fillRect(0, 0, imgW, imgH, frameData->blendColor);
    }
	painter.restore();
}

shine::vFlashThread::vFlashThread(QObject *parent)
	: QThread(parent)
{
}

shine::vFlashThread::~vFlashThread()
{
	quit();
}

void shine::vFlashThread::addFlashItem(const QString &name, const QString & path)
{
	if (name.isEmpty()) {
		return;
	}
	if (path.isEmpty()) {
		return;
	}
	QMutexLocker locker(&m_mutex);
	if (m_images.contains(name)) {
		return;
	}
	auto item = QPair<QString, QString>(name, path);
	m_images[name] = QPixmap();
	m_items.append(item);
}

void shine::vFlashThread::startFlashThread()
{
	m_start = true;
	if (!isRunning()) {
		start();
	}
}

void shine::vFlashThread::stopFlashThread()
{
	m_start = false;
	QMutexLocker locker(&m_mutex);
	m_items.clear();
	m_images.clear();
	locker.unlock();
	quit();
}

const QPixmap shine::vFlashThread::getImage(const QString& name)
{
	QMutexLocker locker(&m_mutex);
	if (!m_images.contains(name)) {
		return QPixmap();
	}
	return m_images.value(name);
}

void shine::vFlashThread::run()
{
	for (auto i = 0; i < m_items.size() && m_start; i++) {
		QMutexLocker locker(&m_mutex);
		auto item = m_items.value(i);
		auto textName = item.first;
		auto path = item.second;
		if (textName.isEmpty()) {
			continue;
		}
		if (path.isEmpty()) {
			continue;
		}
		//读取图片资源
		if (m_images.contains(textName)) {
			auto img = m_images.value(textName);
			if (img.isNull()) {
				try {
					m_images[textName] = QPixmap::fromImage(QImage(path));
				}
				catch (const std::exception& e) {
					qInfo() << "[Flash]" << e.what();
				}
			}
		}
	}
}
