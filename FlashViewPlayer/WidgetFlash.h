#pragma once

#include <QWidget>
#include "FlashView.h"

class WidgetFlash : public shine::FlashView
{
	Q_OBJECT

public:
	WidgetFlash(QWidget *parent = Q_NULLPTR);
	~WidgetFlash();

	void playFlashByDownLoad(const QString &name, const QString &flashPath, const QString &flashUrl, float scale = 0.5);
	void playFlashByLocalPath(const QString &flashName, const QString &flashPath, float scale = 0.5, int loop = 0);
	void stop();

signals:
	void sigFlashFinsh();

private:
	QString m_current;
};
