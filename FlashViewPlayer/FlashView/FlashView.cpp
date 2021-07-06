#include <QPainter>
#include "FlashView.h"
#include "FlashDataParser.h"
#include <QtDebug>
#include <QDateTime>

shine::FlashView::FlashView(QWidget * parent) : QWidget(parent)
{
	mlastUpdateTime = 0;
	mCurrUpdateTime = 0;
	mDataParser.setEventCallback(std::bind(&FlashView::callback, this, std::placeholders::_1, std::placeholders::_2));
	connect(&m_TimerUpdate, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
}

shine::FlashView::~FlashView()
{
	m_TimerUpdate.stop();
}

bool shine::FlashView::reload(const std::vector<std::string>& flashName, const std::string & flashDir, float scale)
{
	return mDataParser.reload(flashName, flashDir, scale);
}

bool shine::FlashView::reload(const std::string & flashName, const std::string & flashDir, float scale)
{
	return  mDataParser.reload(std::vector<std::string>{ flashName }, flashDir, scale);
}

void shine::FlashView::reload(const std::string & flashName, const std::string & flashDir, const std::string & url, downloaded cb, float scale)
{
	mUrl = url;
	m_TimerUpdate.stop();
	if (mDataParser.reload(std::vector<std::string>({ flashName }), flashDir, scale)) {
		if (cb) cb(true);
	}
	else {
		mDownloader.setDownloadFlashDir(flashDir);
		size_t found = flashDir.find_last_of("/\\");
		std::string tmp;
		if (std::string::npos != found)
		{
			tmp = flashDir.substr(0, found);
		}
		mDownloader.setDownloadFlashZipDir(tmp + "/tmp");
		mDownloader.downloadAnimFile(url, flashName, false, [=](bool ret) {
			if (ret) {
				ret = mDataParser.reload(std::vector<std::string>({ flashName }), flashDir, scale);
			}
			if (cb) cb(ret);
		});
	}
}

bool shine::FlashView::play(const std::vector<std::string>& animName, int loopTimes, int fromIndex, int toIndex)
{
	if (!mDataParser.isInitOk()) {
		qDebug() << "[Error] data parser is not init ok:" + QString::fromStdString(animName[0]);
		return false;
	}
	if (-1 == toIndex) {
		toIndex = mDataParser.getParseFrameMaxIndex();
	}
	mlastUpdateTime = 0;
	mCurrUpdateTime = 0;
	mDataParser.play(animName, loopTimes, fromIndex, toIndex); 
	m_TimerUpdate.start((int)(mDataParser.getOneFrameTime() * 1000));
	//qDebug() <<  "FlashView-play " + QString::fromStdString(animName[0]);
	return true;
}

bool shine::FlashView::play(const std::vector<std::string>& animName, int loopTimes, int fromIndex) {
	return play(animName, loopTimes, fromIndex, mDataParser.getParseFrameMaxIndex());
}

bool shine::FlashView::play(const std::vector<std::string>& animName, int loopTimes) {
	return play(animName, loopTimes, 0);
}

bool shine::FlashView::play(std::string & animName, int loopTimes) {
	return play(std::vector<std::string>{animName}, loopTimes);
}

bool shine::FlashView::play(std::string & animName) {
	return play(std::vector<std::string>{animName}, 1);
}

void shine::FlashView::setEventCallback(const FlashViewCallback &callback)
{
	if (nullptr == callback) {
		mDataParser.setEventCallback(std::bind(&FlashView::callback, this, std::placeholders::_1, std::placeholders::_2));
	}
	else 
	{
		mDataParser.setEventCallback(callback);
	}
	
}

void shine::FlashView::stopWithIndex(int index)
{
	mDataParser.stopWithIndex(index);
}


void shine::FlashView::setExtraScale(float * x, float * y)
{
	mDataParser.setExtraScale(x, y);
}

void shine::FlashView::setExtraScale(float x, float y)
{
	setExtraScale(new float[1]{ x }, new float[1]{ y });
}

QString shine::FlashView::getImageFileName(int index)
{
	return mDataParser.getImageFileName(index);
}

void shine::FlashView::setFlashZipUrl(const std::string & url)
{
	mUrl = url;
}

void shine::FlashView::paintEvent(QPaintEvent *event)
{
	if (!mDataParser.isInitOk()) {
		return;
	}
	Q_UNUSED(event);
	if (mlastUpdateTime == 0) {
		mlastUpdateTime = QDateTime::currentMSecsSinceEpoch();
		mDataParser.handleStart();
	}
	mCurrUpdateTime = QDateTime::currentMSecsSinceEpoch();
	mDataParser.increaseTotalTime((mCurrUpdateTime - mlastUpdateTime) / 1000.f);
	mlastUpdateTime = mCurrUpdateTime;
	QPainter painter(this);
	// ·´×ßÑù(·À¾â³Ý)
	//painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	if (!mDataParser.draw(painter) && mDataParser.getRunningAnimName().size() != 0) {
		mDataParser.stopAtLastIndex(painter);
	}
}

void shine::FlashView::callback(shine::FlashViewEvent e, shine::FlashViewEventData * data)
{
	emit onEvent(e, data);
}


