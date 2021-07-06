#pragma once

#include <QtWidgets/QWidget>
#include "ui_FlashViewPlayer.h"

class FlashViewPlayer : public QWidget
{
    Q_OBJECT

public:
    FlashViewPlayer(QWidget *parent = Q_NULLPTR);
	~FlashViewPlayer();

private slots:
	void sltPlayFlash();
	void sltPlayNetWorkFlash();
	void sltClearFlash();

private:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void closeEvent(QCloseEvent *event);

private:
    Ui::FlashViewPlayerClass ui;
	//窗口移动属性值
	QPoint m_point;
	volatile bool m_bMove = false;
};
