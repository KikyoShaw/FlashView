#pragma once

#include <iostream>
#include <functional>
#include <QThread>
#include <unordered_map>
#include <vector>
#include <QEventLoop>
#include <QtNetwork/QtNetwork>



namespace shine {

	class FlashViewDownloader;

	class DownloaderThread : public QThread
	{
		Q_OBJECT
	private:
		volatile bool mStopped = false;
		QNetworkAccessManager * mNetworkManager = nullptr;
		QEventLoop *mEventLoop = nullptr;
	public:
		DownloaderThread(QObject *parent = nullptr);
		~DownloaderThread();
		void stop();

	private:
		void fileDownloaded(QNetworkReply* reply);
		bool unzipFile(const std::string& zipPath, const std::string& destPath);
		std::string basename(const std::string& path);
		std::string UTF8StringToMultiByte(const std::string& strUtf8);
		std::wstring StringUtf8ToWideChar(const std::string & strUtf8);
	protected:
		void run();


	};

	

	typedef std::function<void(bool)> downloaded;

	class FlashViewDownloader: public QObject
	{
		Q_OBJECT
	private:
		static DownloaderThread Thread;
		std::string mDownloadFlashDir;
		std::string mDownloadFlashZipDir;
		std::string mFlashFileName;
		std::string mUrl;
	public:
		downloaded Cabllback = nullptr;
	public:
		FlashViewDownloader(QObject *parent = nullptr);
		~FlashViewDownloader();
		/***
		* ��������Ŀ��flash�ļ�Ŀ¼
		* @param flashDir
		*/
		void setDownloadFlashDir(const std::string & flashDir);
		void setDownloadFlashZipDir(const std::string & zipDir);
		std::string getDownloadFlashDir();
		std::string getDownloadFlashZipDir();
		std::string getFlashFileName();
		/*
		 *����flash����
		 *url
		 *   �������ӵ�ַ
		 *fileName
		 *   �����ļ�flajson�����֣�ʡ�Ժ�׺.flajson
		 *forceUpdate
		 *   �Ƿ�ǿ�Ƹ��£��ж�������fileName�ļ��Ƿ���ڣ�����fileName������ȷ��������
		 *cabllback
		 *   ������ɻص�
		 */
		void downloadAnimFile(const std::string & url, const std::string & fileName, bool forceUpdate, downloaded cabllback);

		
	};


}


