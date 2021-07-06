#pragma once
#include <QWidget>
#include <QTimer>
#include "FlashDataParser.h"
#include "FlashViewDownloader.h"

namespace shine {

	class FlashView :public QWidget
	{
		Q_OBJECT

		QTimer m_TimerUpdate;
		FlashDataParser mDataParser;
		FlashViewDownloader mDownloader;
		qint64 mlastUpdateTime;
		qint64 mCurrUpdateTime;
		std::string mUrl;
	public:
		FlashView(QWidget *parent = nullptr);
		~FlashView();
		/**
		*flashName
		*      flash文件列表
		*flashDir
		*      flash根目录
		*scale
		*      缩放比例
		*/
		bool reload(const std::vector<std::string> &flashName, const std::string &flashDir, float scale = 1.f);
		bool reload(const std::string &flashName, const std::string &flashDir, float scale = 1.f);
		/**
		*flashName
		*      flash文件名称
		*flashDir
		*      flash根目录
		*url 
		*      flash动画网络下载地址
		*cb
		*      加载完成回调
		*scale
		*      缩放比例
		*/
		void reload(const std::string &flashName, const std::string &flashDir, const std::string & url, downloaded cb, float scale = 1.f);
		/**
		*animName
		*     动画名字列表
		*loopTimes
		*      播放次数，0为无限循环
		*fromIndex
		*      开始播放的帧序号
		*toIndex
		*      结束播放的帧序号， -1为播放到末尾
		*/
		bool play(const std::vector<std::string> & animName, int loopTimes, int fromIndex, int toIndex);
		bool play(const std::vector<std::string>& animName, int loopTimes, int fromIndex);
		bool play(const std::vector<std::string>& animName, int loopTimes);
		bool play(std::string & animName, int loopTimes);
		bool play(std::string & animName);
		/*
		*flash事件回调
		*/
		void setEventCallback(const FlashViewCallback &callback);
		/*
		*
		* 最后停到哪个界面
		* 0是第一帧
		* -1是最后一帧
		*
		* */
		void stopWithIndex(int index);
		/*
		*额外缩放
		*/
		void setExtraScale(float* x, float* y);
		void setExtraScale(float x, float y);
		/*
		*根据帧获取当前图片
		*/
		QString getImageFileName(int index);
		/*
		*加载失败是是否使用url下载
		*/
		void setFlashZipUrl(const std::string & url);
	signals:
		void onEvent(FlashViewEvent e, FlashViewEventData * data);
		
	protected:
		void paintEvent(QPaintEvent *event) override;
		void callback(shine::FlashViewEvent e, shine::FlashViewEventData *data);

	
	};

}


