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
		*      flash�ļ��б�
		*flashDir
		*      flash��Ŀ¼
		*scale
		*      ���ű���
		*/
		bool reload(const std::vector<std::string> &flashName, const std::string &flashDir, float scale = 1.f);
		bool reload(const std::string &flashName, const std::string &flashDir, float scale = 1.f);
		/**
		*flashName
		*      flash�ļ�����
		*flashDir
		*      flash��Ŀ¼
		*url 
		*      flash�����������ص�ַ
		*cb
		*      ������ɻص�
		*scale
		*      ���ű���
		*/
		void reload(const std::string &flashName, const std::string &flashDir, const std::string & url, downloaded cb, float scale = 1.f);
		/**
		*animName
		*     ���������б�
		*loopTimes
		*      ���Ŵ�����0Ϊ����ѭ��
		*fromIndex
		*      ��ʼ���ŵ�֡���
		*toIndex
		*      �������ŵ�֡��ţ� -1Ϊ���ŵ�ĩβ
		*/
		bool play(const std::vector<std::string> & animName, int loopTimes, int fromIndex, int toIndex);
		bool play(const std::vector<std::string>& animName, int loopTimes, int fromIndex);
		bool play(const std::vector<std::string>& animName, int loopTimes);
		bool play(std::string & animName, int loopTimes);
		bool play(std::string & animName);
		/*
		*flash�¼��ص�
		*/
		void setEventCallback(const FlashViewCallback &callback);
		/*
		*
		* ���ͣ���ĸ�����
		* 0�ǵ�һ֡
		* -1�����һ֡
		*
		* */
		void stopWithIndex(int index);
		/*
		*��������
		*/
		void setExtraScale(float* x, float* y);
		void setExtraScale(float x, float y);
		/*
		*����֡��ȡ��ǰͼƬ
		*/
		QString getImageFileName(int index);
		/*
		*����ʧ�����Ƿ�ʹ��url����
		*/
		void setFlashZipUrl(const std::string & url);
	signals:
		void onEvent(FlashViewEvent e, FlashViewEventData * data);
		
	protected:
		void paintEvent(QPaintEvent *event) override;
		void callback(shine::FlashViewEvent e, shine::FlashViewEventData *data);

	
	};

}


