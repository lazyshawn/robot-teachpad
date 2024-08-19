#pragma once

#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <chrono>

#include "fsai_mainwindow.h"
#include "fsai_procedure.h"
#include "fsai_advanceConfigWindow.h"

#include "zmotion_interface.h"

class RobotStatus {
public:
	bool online = false;
	int mode;
	std::vector<float> jPos, cPos;
};

/**
* @brief  扫描定时器
*/
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
	// 窗口
	std::shared_ptr<MainWindow> mainWindow;
	std::shared_ptr<ProcedureWindow> procedureWindow;
	std::shared_ptr<AdvanceConfigWindow> advanceConfigWindow;

	std::shared_ptr<ZmotionScanner> zScanner;
	std::shared_ptr<QThread> zScannerThread;

	std::shared_ptr<ZauxRobot> robot;
	std::shared_ptr<RobotStatus> robotStatus;

	std::vector<TrajectoryConfig<float>> procData;

public:
	FSAIApp();
	~FSAIApp();

	int init_connect_module();
	int init_jog_module();
	int init_teach_module();
	int init_menu_module();
	int init_monitor_module();
	int init_log_module();
	int init_control_module();

	int connect_robot();
	int disconnect_robot();

	std::vector<float> read_teach_point(int row, const std::vector<float>& idxList) const;
	// 工艺参数
	void load_procedure_param(int idx);
	void save_current_procedure_param();
	// 配置参数
	void load_advance_config();
	void save_advance_config();
	void update_speedRatio();

public slots:
	void switch_online();
	void refresh_monitor();
	void excute_selected_teach_point();
	void start_teach_trajectory() const;
	void switch_auto(bool autoMode);
};

