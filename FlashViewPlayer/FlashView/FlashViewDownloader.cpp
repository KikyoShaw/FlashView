#include <mutex>
#include "FlashViewDownloader.h"
#include "unzip.h"
#include "zlib.h"
#include <windows.h>
#include <QMutexLocker>

using namespace shine;
using namespace std;

#define BUFFER_SIZE    8192
#define MAX_FILENAME   512

QMutex mtx;
unordered_map<string, vector<QPointer<FlashViewDownloader> >*> Tasks;

DownloaderThread FlashViewDownloader::Thread;

FlashViewDownloader::FlashViewDownloader(QObject *parent):QObject(parent)
{
}


FlashViewDownloader::~FlashViewDownloader()
{
	QMutexLocker locker(&mtx);
	auto loaders = Tasks[mUrl];
	if (nullptr != loaders) {
		for (int i = 0, c = loaders->size(); i < c; i++) {
			auto loader = (*loaders)[i];
			if (loader == this) {
				loaders->erase(loaders->begin() + i);
				break;
			}
		}
	}
	else {
		// 注意：map的操作符[]可能插入空指针，所以可以删掉，否则引起后续流程取到空指针而访问异常
		Tasks.erase(mUrl);
	}
	
}

void FlashViewDownloader::setDownloadFlashDir(const string & flashDir)
{
	mDownloadFlashDir = flashDir;
}

void FlashViewDownloader::setDownloadFlashZipDir(const string & zipDir)
{
	mDownloadFlashZipDir = zipDir;
}

string FlashViewDownloader::getDownloadFlashDir()
{
	return mDownloadFlashDir;
}

string FlashViewDownloader::getDownloadFlashZipDir()
{
	return mDownloadFlashZipDir;
}

string FlashViewDownloader::getFlashFileName()
{
	return mFlashFileName;
}

void FlashViewDownloader::downloadAnimFile(const string & url,const string & fileName, bool forceUpdate, downloaded cabllback)
{	
	mFlashFileName = fileName;
	Cabllback = cabllback;
	mUrl = url;
	QFile file(QString::fromStdString(mDownloadFlashDir + "/" + fileName + ".flajson"));
	if (!forceUpdate && file.exists()) {
		if (cabllback != nullptr) cabllback(true);
		return;
	}
	if (!forceUpdate && file.exists()) {
		if (cabllback != nullptr) cabllback(true);
		return;
	}
	QMutexLocker locker(&mtx);
	// 删除动画模块下载缓存中的空白地址项，从而避免重新下载有时出现暂停
	if (Tasks.find("") != Tasks.end())
	{
		Tasks.erase("");
	}
	bool needStart = (Tasks.size() == 0);
	//保存弱引用
	QPointer<FlashViewDownloader> item(this);
	if (Tasks.find(url) != Tasks.end()) {
		Tasks[url]->push_back(item);
	}
	else {
		auto list = new vector<QPointer<FlashViewDownloader> >();
		list->push_back(item);
		Tasks[url] = list;
	}
	locker.unlock();
	if (needStart) Thread.start();
}

DownloaderThread::DownloaderThread(QObject * parent)
{
}

DownloaderThread::~DownloaderThread()
{
	stop();
	if (nullptr != mNetworkManager) {
		mNetworkManager->deleteLater();
	}
	
}

void DownloaderThread::stop()
{
	mStopped = true;
	if (mEventLoop != nullptr) {
		mEventLoop->exit();
		mEventLoop->deleteLater();
	}
	
}


void DownloaderThread::run()
{
	if (nullptr == mNetworkManager) {
		mNetworkManager = new QNetworkAccessManager(this);
		connect(mNetworkManager, &QNetworkAccessManager::finished, this, &DownloaderThread::fileDownloaded);
	}
	QNetworkRequest request;
	QSslConfiguration conf = request.sslConfiguration();
	conf.setPeerVerifyMode(QSslSocket::VerifyNone);
	conf.setProtocol(QSsl::TlsV1SslV3);
	request.setSslConfiguration(conf);
	//int i = 0;
	while (!mStopped && Tasks.size() > 0)
	{
		for (auto pair : Tasks) {
			request.setUrl(QString::fromStdString(pair.first));
			mNetworkManager->get(request);
			break;
		}
		if (mEventLoop == nullptr) {
			mEventLoop = new QEventLoop;
		}
		mEventLoop->exec();
		//qDebug() << "while" + QString::number(i++);
	}
}

void DownloaderThread::fileDownloaded(QNetworkReply * reply)
{
	bool ret = true;
	QMutexLocker locker(&mtx);
	string url = reply->url().url().toStdString();
	// 删除动画模块下载缓存中的空白地址项，这里避免异常和冗余
	if (url.empty()) {
		Tasks.erase("");
		locker.unlock();
		mEventLoop->exit();
		return;
	}
	// 可能已经删掉这个任务（同时重复下载）
	if (Tasks.find(url) == Tasks.end()) {
		locker.unlock();
		mEventLoop->exit();
		return;
	}
	auto loaders = Tasks[url];
	//判断列表是否为空
	if (loaders == Q_NULLPTR) {
		Tasks.erase(url);
		locker.unlock();
		mEventLoop->exit();
		return;
	}
	if (loaders->empty()) {
		Tasks.erase(url);
		locker.unlock();
		mEventLoop->exit();
		return;
	}
	//判断弱引用是否删除
	auto loader = (*loaders)[0];
	if (loader.isNull()) {
		qInfo() << __FUNCTION__ << "error download" << QString::fromStdString(url);
		Tasks.erase(url);
		locker.unlock();
		mEventLoop->exit();
		return;
	}
	//解压保存
	if (reply->error() == QNetworkReply::NoError) {
		QDir dir;
		QString zipDair = QString::fromStdString(loader->getDownloadFlashZipDir());
		if (!dir.exists(zipDair)) {
			dir.mkpath(zipDair);
		}
		string fullPath = loader->getDownloadFlashZipDir() + "/" + loader->getFlashFileName();
		QFile file(QString::fromStdString(fullPath));
		if (file.open(QIODevice::WriteOnly))
		{
			file.write(reply->readAll());
			file.close();
			reply->deleteLater();
			QString  root = QString::fromStdString(loader->getDownloadFlashDir());
			if (!dir.exists(root)) {
				dir.mkpath(root);
			}
			ret = unzipFile(fullPath, loader->getDownloadFlashDir());
			if (ret) {
				QString oldName = QString::fromStdString(loader->getDownloadFlashDir() + "/__" + loader->getFlashFileName() + ".flajson");
				QString newName = QString::fromStdString(loader->getDownloadFlashDir() + "/" + loader->getFlashFileName() + ".flajson");
				QFile::rename(oldName, newName);

				// 删除压缩包，用于清理
				{
					QString strFullPath = QString::fromStdString(fullPath);
					QFile file(strFullPath);
					bool bRemove = file.remove();
					if (!bRemove)
					{
						qInfo() << QStringLiteral("tidy flash unZipFile fail") << strFullPath;
					}
				}
			}
			
		}
		else {
			ret = false;
			qDebug() << QString::fromStdString("open fail" + loader->getDownloadFlashZipDir() + "/" + loader->getFlashFileName());
		}
	}
	else {
		ret = false;
		qDebug() << QString::fromStdString("network fail" + loader->getDownloadFlashZipDir() + "/" + loader->getFlashFileName());
	}
	Tasks.erase(url);
	locker.unlock();
	for (int i = 0, c = loaders->size(); i < c; i++) {
		auto loaderObj = (*loaders)[i];
		if (loaderObj.isNull()) {
			continue;
		}
		downloaded cb = loaderObj->Cabllback;
		if (cb != nullptr) cb(ret);
	}
	delete loaders;
	mEventLoop->exit();
}

bool DownloaderThread::unzipFile(const string & zipPath, const string & destPath)
{
	//打开zip文件
	unzFile zipfile = unzOpen(zipPath.c_str());
	if (!zipfile)
	{
		qDebug() << QString::fromStdString("can not open downloaded zip file " + zipPath);
		return false;
	}

	// Get info about the zip file
	unz_global_info global_info;
	if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
	{
		qDebug() << QString::fromStdString("can not read file global info of " + zipPath);
		unzClose(zipfile);
		return false;
	}
	QDir qDir;
	// Buffer to hold data read from the zip file
	char readBuffer[BUFFER_SIZE];
	// Loop to extract all files.
	uLong i;
	for (i = 0; i < global_info.number_entry; ++i)
	{
		// Get info about current file.
		unz_file_info fileInfo;
		char fileName[MAX_FILENAME];
		if (unzGetCurrentFileInfo(zipfile,
			&fileInfo,
			fileName,
			MAX_FILENAME,
			NULL,
			0,
			NULL,
			0) != UNZ_OK)
		{
			qDebug() << QString::fromStdString("can not read compressed file info");
			unzClose(zipfile);
			return false;
		}
		const string fullPath = destPath + "/" + fileName;

		// Check if this entry is a directory or a file.
		const size_t filenameLength = strlen(fileName);
		if (fileName[filenameLength - 1] == '/')
		{
			//There are not directory entry in some case.
			//So we need to create directory when decompressing file entry
			if (!qDir.mkpath(QString::fromStdString(basename(fullPath))))
			{
				// Failed to create directory
				qDebug() << QString::fromStdString("can not create directory " + fullPath);
				unzClose(zipfile);
				return false;
			}
		}
		else
		{
			// Create all directories in advance to avoid issue
			QString dir = QString::fromStdString(basename(fullPath));
			if (!qDir.exists(dir)) {
				if (!qDir.mkpath(dir)) {
					// Failed to create directory
					qDebug() << QString::fromStdString(" can not create directory " + fullPath);
					unzClose(zipfile);
					return false;
				}
			}
			// Entry is a file, so extract it.
			// Open current file.
			if (unzOpenCurrentFile(zipfile) != UNZ_OK)
			{
				qDebug() << "can not extract file " + QString(fileName);
				unzClose(zipfile);
				return false;
			}

			// Create a file to store current file.
			FILE *out = fopen(UTF8StringToMultiByte(fullPath).c_str(), "wb");
			if (!out)
			{
				qDebug() << QString::fromStdString("can not create decompress destination file " + fullPath + "(errno: " + to_string(errno)  + ")");
				unzCloseCurrentFile(zipfile);
				unzClose(zipfile);
				return false;
			}

			// Write current file content to destinate file.
			int error = UNZ_OK;
			do
			{
				error = unzReadCurrentFile(zipfile, readBuffer, BUFFER_SIZE);
				if (error < 0)
				{
					qDebug() << "can not read zip file " + QString(fileName)  + ", error code is " + QString::fromStdString(to_string(error));
					fclose(out);
					unzCloseCurrentFile(zipfile);
					unzClose(zipfile);
					return false;
				}

				if (error > 0)
				{
					fwrite(readBuffer, error, 1, out);
				}
			} while (error > 0);

			fclose(out);
		}

		unzCloseCurrentFile(zipfile);

		// Goto next entry listed in the zip file.
		if ((i + 1) < global_info.number_entry)
		{
			if (unzGoToNextFile(zipfile) != UNZ_OK)
			{
				qDebug() << "can not read next file for decompressing";
				unzClose(zipfile);
				return false;
			}
		}
	}

	unzClose(zipfile);
	return true;

}

string DownloaderThread::basename(const string & path)
{
	size_t found = path.find_last_of("/\\");

	if (string::npos != found)
	{
		return path.substr(0, found);
	}
	else
	{
		return path;
	}
}

string DownloaderThread::UTF8StringToMultiByte(const string& strUtf8)
{
	string ret;
	if (!strUtf8.empty())
	{
		wstring strWideChar = StringUtf8ToWideChar(strUtf8);
		int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* ansiString = new char[nNum + 1];
			ansiString[0] = 0;

			nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

			ret = ansiString;
			delete[] ansiString;
		}
		else
		{
			qDebug() << QString::fromStdString("Wrong convert to Ansi code:0x" + to_string(GetLastError()));
		}
	}

	return ret;
}

wstring DownloaderThread::StringUtf8ToWideChar(const string& strUtf8)
{
	wstring ret;
	if (!strUtf8.empty())
	{
		int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (nNum)
		{
			WCHAR* wideCharString = new WCHAR[nNum + 1];
			wideCharString[0] = 0;

			nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

			ret = wideCharString;
			delete[] wideCharString;
		}
		else
		{
			qDebug() << QString::fromStdString("Wrong convert to WideChar code:0x" + to_string(GetLastError()));
		}
	}
	return ret;
}
