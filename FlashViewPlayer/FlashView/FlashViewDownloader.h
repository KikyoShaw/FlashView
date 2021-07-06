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
		* 设置下载目标flash文件目录
		* @param flashDir
		*/
		void setDownloadFlashDir(const std::string & flashDir);
		void setDownloadFlashZipDir(const std::string & zipDir);
		std::string getDownloadFlashDir();
		std::string getDownloadFlashZipDir();
		std::string getFlashFileName();
		/*
		 *下载flash动画
		 *url
		 *   网络连接地址
		 *fileName
		 *   动画文件flajson的名字，省略后缀.flajson
		 *forceUpdate
		 *   是否强制更新，判断依据是fileName文件是否存在，所以fileName必须正确才起作用
		 *cabllback
		 *   下载完成回调
		 */
		void downloadAnimFile(const std::string & url, const std::string & fileName, bool forceUpdate, downloaded cabllback);

		
	};


}


