#pragma once

#include <QThread>
#include <chrono>

#include "fsai_mainwindow.h"
#include "zmotion_interface.h"

class RobotStatus {
public:
	bool online = false;
	int mode;
	std::vector<float> jPos, cPos;
};


class ZmotionScanner :public QObject {
	std::shared_ptr<ZauxRobot> robot;
	std::shared_ptr<RobotStatus> status;

	uint8_t scannerStatus;

Q_OBJECT
public:
	ZmotionScanner(std::shared_ptr<RobotStatus> status_, std::shared_ptr<ZauxRobot> robot_);

public slots:
	void start();
	void pause();
	void resume();
	void stop();

signals:
	void trigger_update();
};



class FSAIApp : public QMainWindow {
Q_OBJECT

private:
	std::shared_ptr<MainWindow> mainWindow;
	std::shared_ptr<ZauxRobot> robot;
	std::shared_ptr<RobotStatus> robotStatus;

	std::shared_ptr<ZmotionScanner> zScanner;
	std::shared_ptr<QThread> zScannerThread;

public:
	FSAIApp();
	~FSAIApp();

	int init_connect_module();
	int init_jog_module();
	int init_teach_module();
	int init_menu_module();
	int init_monitor_module();
	int init_log_module();

	int connect_robot();
	int disconnect_robot();

public slots:
	void switch_connection();
	void refresh_monitor();
};

