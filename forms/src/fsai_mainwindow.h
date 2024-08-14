#pragma once

#include <memory>
#include "mainWindow_uic.h"


/**
* @brief  主窗口
*/
class MainWindow : public QMainWindow {
public:
	// 点动按钮Label
	const std::vector<std::vector<std::string>> jointName = {
		{"J1", "J2", "J3", "J4", "J5", "J6", "E1", "E2", "E3"},
		{"X", "Y", "Z", "Rx", "Ry", "Rz", "Gx", "Gy", "Gz"},
		{"X", "Y", "Z", "Rx", "Ry", "Rz", "Gx", "Gy", "Gz"}
	};

	// 主页面
	std::shared_ptr<Ui_MainWindow> ui;
	// 点动按钮
	std::vector<QPushButton*> jogMoveBtn = std::vector<QPushButton*>(18, NULL);
	std::vector<QLabel*> jogMoveLabel = std::vector<QLabel*>(9, NULL);

	// ************************ 常量 ************************
	// 点动运动类型文本
	const  std::vector<std::string> jogMoveTypeStr = { "Joint", "World", "Tool" };

private:


public:
	MainWindow();
	virtual ~MainWindow();

private:
	int init_connect_module();
	int init_jog_module();
	int init_teach_module();
	int init_menu_module();
	int init_monitor_module();
	int init_log_module();

Q_OBJECT
public slots:
	void jog_moving(int axis, int type);

	void record_teach_point();
	void delete_teach_point();

};

