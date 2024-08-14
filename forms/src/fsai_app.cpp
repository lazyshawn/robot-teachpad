
#include "fsai_app.h"

#include <iostream>

ZmotionScanner::ZmotionScanner(std::shared_ptr<RobotStatus> status_, std::shared_ptr<ZauxRobot> robot_) {
	status = status_;
	robot = robot_;
}

void ZmotionScanner::start() {
	using namespace std::chrono;
	scannerStatus = 1;

	// 获取当前时间戳
	std::chrono::steady_clock::time_point start = steady_clock::now();
	// 下次唤醒时间
	std::chrono::steady_clock::time_point wakeUpTime = start;

	// 轮询
	long long duration = 200;
	while (scannerStatus > 0) {
		// 更新定时器状态
		if (!status->online) {
			this->pause();
		}

		// 暂停
		if (scannerStatus == 1) {

		}
		// Do something
		else {
			// 读取轴状态
			std::vector<int> axis = robot->jointAxisIdx;
			axis.insert(axis.end(), robot->appAxisIdx.begin(), robot->appAxisIdx.end());
			robot->get_axis_param(axis, (char*)"DPOS", status->jPos);

			axis = robot->tcpPosAxisIdx;
			axis.insert(axis.end(), robot->tcpAngleAxisIdx.begin(), robot->tcpAngleAxisIdx.end());
			axis.insert(axis.end(), robot->appAxisIdx.begin(), robot->appAxisIdx.end());
			robot->get_axis_param(axis, (char*)"DPOS", status->cPos);

			status->mode = robot->get_kinematic_mode();

			// 触发监控面板更新
			trigger_update();
		}

		// 设置下次唤醒时间
		wakeUpTime += std::chrono::milliseconds(duration);
		// 休眠
		auto now = steady_clock::now();
		if ((now - wakeUpTime).count() > 0) {
			wakeUpTime = now;
		}
		else {
			std::this_thread::sleep_until(wakeUpTime);
		}
	}
}

void ZmotionScanner::pause() {
	scannerStatus = 1;
}

void ZmotionScanner::resume() {
	scannerStatus = 2;
}

void ZmotionScanner::stop() {
	scannerStatus = 0;
}

FSAIApp::FSAIApp() {
	// 资源初始化
	robot = std::make_shared<ZauxRobot>();
	robotStatus = std::shared_ptr<RobotStatus>(new RobotStatus);

	mainWindow = std::shared_ptr<MainWindow>(new MainWindow);
	//procedureWindow = std::shared_ptr<ProcedureWindow>(new ProcedureWindow);

	// 显示主页
	mainWindow->show();

	// 连接模块初始化
	init_connect_module();

	init_jog_module();
	init_teach_module();

	// 开始监控
	zScanner = std::make_shared<ZmotionScanner>(robotStatus, robot);
	zScannerThread = std::make_shared<QThread>();
	zScanner->moveToThread(zScannerThread.get());
	QObject::connect(zScannerThread.get(), &QThread::started, zScanner.get(), &ZmotionScanner::start);
	QObject::connect(zScanner.get(), &ZmotionScanner::trigger_update, this, &FSAIApp::refresh_monitor);
	zScannerThread->start();

	/*
	// 禁用控制面板
	mainWindow->ui->groupBox->setEnabled(0);
	mainWindow->ui->groupBox_2->setEnabled(0);
	*/
}

FSAIApp::~FSAIApp() {
	zScanner->stop();
	// 结束监控
	zScannerThread->quit();
	zScannerThread->wait();

	if (robotStatus->online) {
		disconnect_robot();
	}
}

int FSAIApp::init_connect_module() {
	// 连接模块
	QObject::connect(mainWindow->ui->pushButton, &QPushButton::pressed, this, &FSAIApp::switch_connection);

	return 0;
}

int FSAIApp::init_jog_module() {
	// 切换坐标系
	QObject::connect(mainWindow->ui->radioButton, &QRadioButton::clicked, this, [&](){
		zScanner->pause();
		int ret = robot->forward_kinematics(10);
		if (ret != 0) {
			mainWindow->ui->textBrowser->append("Failed");
		}
		zScanner->resume();
	});
	QObject::connect(mainWindow->ui->radioButton_2, &QRadioButton::clicked, this, [&]() {
		zScanner->pause();
		int ret = robot->inverse_kinematics(10);
		if (ret != 0) {
			mainWindow->ui->textBrowser->append("Failed");
		}
		zScanner->resume();
	});	

	// 点动界面
	for (int i = 0; i < 9; ++i) {
		QObject::connect(mainWindow->jogMoveBtn[2 * i], &QPushButton::pressed, this, [&, i]() {
			int mode = robotStatus->mode > 0 ? 0 : 1;
			int ret = robot->jog_moving(i, mode, -1);
			QString axisName = "[" + QString::fromStdString(mainWindow->jogMoveTypeStr[mode]) + "] " + QString::fromStdString(mainWindow->jointName[mode][i]);
			mainWindow->ui->textBrowser->append(axisName + "-");
		});
		QObject::connect(mainWindow->jogMoveBtn[2 * i], &QPushButton::released, this, [&, i]() {
			int mode = robotStatus->mode > 0 ? 0 : 1;
			robot->jog_moving(i, mode, 0);
		});

		QObject::connect(mainWindow->jogMoveBtn[2 * i + 1], &QPushButton::pressed, this, [&, i]() {
			int mode = robotStatus->mode > 0 ? 0 : 1;
			robot->jog_moving(i, mode, 1);
			QString axisName = "[" + QString::fromStdString(mainWindow->jogMoveTypeStr[mode]) + "] " + QString::fromStdString(mainWindow->jointName[mode][i]);
			mainWindow->ui->textBrowser->append(axisName + "+");
		});
		QObject::connect(mainWindow->jogMoveBtn[2 * i + 1], &QPushButton::released, this, [&, i]() {
			int mode = robotStatus->mode > 0 ? 0 : 1;
			robot->jog_moving(i, mode, 0);
		});
	}
	return 0;
}

int FSAIApp::init_teach_module() {
	// 记录示教点
	QObject::connect(mainWindow->ui->pushButton_2, &QPushButton::pressed, this, [&]() {
		// 当前行
		int row = mainWindow->ui->tableWidget->currentRow();

		// 读取当前空间位置
		QString posStr = QString::number(robotStatus->cPos[0]);
		for (size_t i = 1; i < 6; ++i) {
			posStr += ", " + QString::number(robotStatus->cPos[i]);
		}
		QTableWidgetItem* item = new QTableWidgetItem(posStr);
		mainWindow->ui->tableWidget->setItem(row, 3, item);

		// 关节位置
		posStr = QString::number(robotStatus->jPos[0]);
		for (size_t i = 1; i < robot->jointAxisIdx.size(); ++i) {
			posStr += ", " + QString::number(robotStatus->jPos[i]);
		}
		item = new QTableWidgetItem(posStr);
		mainWindow->ui->tableWidget->setItem(row, 4, item);

		// 附加轴位置
		posStr = QString::number(robotStatus->jPos[robot->jointAxisIdx.size() + 1]);
		for (size_t i = robot->jointAxisIdx.size()+1; i < robotStatus->jPos.size(); ++i) {
			posStr += ", " + QString::number(robotStatus->jPos[i]);
		}
		item = new QTableWidgetItem(posStr);
		mainWindow->ui->tableWidget->setItem(row, 5, item);
	});
	return 0;
}

int FSAIApp::init_menu_module() {
	// 打开工艺设置界面
	//QObject::connect_robot(mainWindow->ui->pushButton_8, &QPushButton::pressed, procedureWindow.get(), [&]() {
	//	procedureWindow->show();
	//	procedureWindow->activateWindow();
	//});

	return 0;
}

int FSAIApp::init_monitor_module() {
	return 0;
}

int FSAIApp::init_log_module() {
	return 0;
}

int FSAIApp::connect_robot() {
	mainWindow->ui->textBrowser->append("Connecting to " + mainWindow->ui->comboBox->currentText());
	// 读取控制卡ip
	std::string addr = mainWindow->ui->comboBox->currentText().toStdString();
	int ret = robot->connect(addr);
	if (ret != 0) {
		mainWindow->ui->textBrowser->append("<Zmotion>: connect error " + QString::number(ret));
		return ret;
	}

	// 连接成功
	robotStatus->online = true;
	mainWindow->ui->pushButton->setText("Disconnect");
	zScanner->resume();


	return 0;
}

int FSAIApp::disconnect_robot() {
	mainWindow->ui->textBrowser->append("Disconnect");
	int ret = robot->disconnect();
	if (ret != 0) {
		mainWindow->ui->textBrowser->append("<Zmotion>: connect error " + QString::number(ret));
		return ret;
	}

	// 断开成功
	robotStatus->online = false;
	mainWindow->ui->pushButton->setText("Connect");


	return 0;
}


void FSAIApp::switch_connection() {
	if (robotStatus->online) {
		disconnect_robot();
	}
	else {
		connect_robot();
	}
}


void FSAIApp::refresh_monitor() {
	for (size_t i = 0; i < 9; ++i) {
		QTableWidgetItem* seqItem = new QTableWidgetItem(QString::number(robotStatus->jPos[i]));
		mainWindow->ui->tableWidget_2->setItem(i, 1, seqItem);

		seqItem = new QTableWidgetItem(QString::number(robotStatus->cPos[i]));
		mainWindow->ui->tableWidget_2->setItem(i, 2, seqItem);
	}

	mainWindow->ui->radioButton->setChecked(robotStatus->mode == 1);
	mainWindow->ui->radioButton_2->setChecked(robotStatus->mode == -1);
}

