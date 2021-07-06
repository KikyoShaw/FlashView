#include "FlashViewPlayer.h"
#include <QMouseEvent>

FlashViewPlayer::FlashViewPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	connect(ui.pushButton_close, &QPushButton::clicked, this, &QWidget::close);
	connect(ui.pushButton_play, &QPushButton::clicked, this, &FlashViewPlayer::sltPlayFlash);
	connect(ui.pushButton_urlPlay, &QPushButton::clicked, this, &FlashViewPlayer::sltPlayNetWorkFlash);
	connect(ui.pushButton_clear, &QPushButton::clicked, this, &FlashViewPlayer::sltClearFlash);
}

FlashViewPlayer::~FlashViewPlayer()
{
}

void FlashViewPlayer::sltPlayFlash()
{
	QString flashName = "pczsskzscg01";
	ui.widget_flash->playFlashByLocalPath(flashName, flashName, 1);
}

void FlashViewPlayer::sltPlayNetWorkFlash()
{
	QString flashName = "yrjxcmvlwxg";
	QString falshFlash = "https://image-app-test.jiaoyoushow.com//app/logo/giftanimation/yrjxcmvlwxg.zip?1618386011";
	ui.widget_flash->playFlashByDownLoad(flashName, flashName, falshFlash, 1);
}

void FlashViewPlayer::sltClearFlash()
{
	ui.widget_flash->stop();
}

void FlashViewPlayer::mouseMoveEvent(QMouseEvent * event)
{
	//判断左键是否被按下，只有左键按下了，其返回值就是1(true)
	if ((event->buttons() & Qt::LeftButton) && m_bMove)
	{
		move(event->globalPos() - m_point);
	}
	QWidget::mouseMoveEvent(event);
}

void FlashViewPlayer::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_bMove = true;
		m_point = event->globalPos() - frameGeometry().topLeft();
	}
	QWidget::mousePressEvent(event);
}

void FlashViewPlayer::mouseReleaseEvent(QMouseEvent * event)
{
	m_bMove = false;
	QWidget::mouseReleaseEvent(event);
}

void FlashViewPlayer::closeEvent(QCloseEvent * event)
{
	ui.widget_flash->stop();
	QWidget::closeEvent(event);
}
