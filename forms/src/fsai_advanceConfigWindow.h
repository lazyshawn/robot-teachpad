#pragma once

#include <QDialog>

#include "advanceConfigWindow_uic.h"

class AdvanceConfigWindow : public QDialog {
public:
	// ��ҳ��
	std::shared_ptr<Ui_AdvanceConfigWindow> ui;

private:


public:
	AdvanceConfigWindow();
	virtual ~AdvanceConfigWindow();


private:

	Q_OBJECT
public slots:
	// void switch_online();
	// void jog_moving(int axis, int type);

};
