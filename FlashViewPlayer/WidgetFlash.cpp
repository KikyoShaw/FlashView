#include "WidgetFlash.h"
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

using namespace shine;

WidgetFlash::WidgetFlash(QWidget *parent)
	: FlashView(parent)
{
	connect(this, &FlashView::onEvent, this, [=](FlashViewEvent event, FlashViewEventData * data){
		if (FlashViewEvent::STOP == event) {
			emit sigFlashFinsh();
		}
	});
}

WidgetFlash::~WidgetFlash()
{
}

void WidgetFlash::playFlashByDownLoad(const QString & name, const QString &flashPath, const QString &flashUrl, float scale)
{
	if (flashPath == m_current) {
		return;
	}
	reload(name.toStdString(), flashPath.toStdString(), flashUrl.toStdString(), [=](bool ret) {
		if (ret) {
			if (play(name.toStdString(), 0)) {
				m_current = flashPath;
				show();
				return;
			}
		}
	}, scale);
}

void WidgetFlash::playFlashByLocalPath(const QString &flashName, const QString & flashPath, float scale, int loop/* = 0*/)
{
	if (flashPath == m_current) {
		return;
	}
	bool flag = reload(flashName.toStdString(), flashPath.toStdString(),scale);
	if (flag) {
		if (play(flashName.toStdString(), loop)) {
			m_current = flashPath;
			show();
			return;
		}
	}
	else {
		qInfo() << QStringLiteral("播放本地动画失败!");
	}
}

void WidgetFlash::stop()
{
	m_current.clear();
	hide();
	//释放内存
	reload("", "");
}
