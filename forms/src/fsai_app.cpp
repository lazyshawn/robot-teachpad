
#include "fsai_app.h"

#include <iostream>

/**
* @brief  扫描定时器
*/
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



/**
* @brief  主窗口
*/
FSAIApp::FSAIApp() {
	// 资源初始化
	robot = std::make_shared<ZauxRobot>();
	robotStatus = std::shared_ptr<RobotStatus>(new RobotStatus);

	mainWindow = std::shared_ptr<MainWindow>(new MainWindow);
	procedureWindow = std::shared_ptr<ProcedureWindow>(new ProcedureWindow);
	advanceConfigWindow = std::shared_ptr<AdvanceConfigWindow>(new AdvanceConfigWindow);

	// 显示主页
	mainWindow->show();

	// 连接模块初始化
	init_connect_module();

	init_jog_module();
	init_teach_module();
	init_menu_module();
	init_control_module();

	// 开始监控
	zScanner = std::make_shared<ZmotionScanner>(robotStatus, robot);
	zScannerThread = std::make_shared<QThread>();
	zScanner->moveToThread(zScannerThread.get());
	QObject::connect(zScannerThread.get(), &QThread::started, zScanner.get(), &ZmotionScanner::start);
	QObject::connect(zScanner.get(), &ZmotionScanner::trigger_update, this, &FSAIApp::refresh_monitor);
	zScannerThread->start();

	// 初始化控制面板
	mainWindow->refresh_display_switch_online(false);
}

FSAIApp::~FSAIApp() {
	// 结束监控
	zScanner->stop();
	zScannerThread->quit();
	zScannerThread->wait();

	if (robotStatus->online) {
		disconnect_robot();
	}
}

// 初始化
int FSAIApp::init_connect_module() {
	// 连接模块
	QObject::connect(mainWindow->ui->pushButton, &QPushButton::pressed, this, &FSAIApp::switch_online);

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
		// 更新速度
		update_speedRatio();
	});
	QObject::connect(mainWindow->ui->radioButton_2, &QRadioButton::clicked, this, [&]() {
		zScanner->pause();
		int ret = robot->inverse_kinematics(10);
		if (ret != 0) {
			mainWindow->ui->textBrowser->append("Failed");
		}
		zScanner->resume();
		// 更新速度
		update_speedRatio();
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
		posStr = QString::number(robotStatus->jPos[robot->jointAxisIdx.size()]);
		for (size_t i = robot->jointAxisIdx.size() + 1; i < robotStatus->jPos.size(); ++i) {
			posStr += ", " + QString::number(robotStatus->jPos[i]);
		}
		item = new QTableWidgetItem(posStr);
		mainWindow->ui->tableWidget->setItem(row, 5, item);
	});

	// 执行选中点
	QObject::connect(mainWindow->ui->pushButton_7, &QPushButton::pressed, this, &FSAIApp::excute_selected_teach_point);
	// 执行示教轨迹
	//QObject::connect(mainWindow->ui->pushButton_4, &QPushButton::pressed, this, &FSAIApp::start_teach_trajectory);
	QObject::connect(mainWindow->ui->pushButton_4, &QPushButton::pressed, this, [&]() {
		QtConcurrent::run(this, &FSAIApp::start_teach_trajectory);
		//std::thread thread = std::thread(&FSAIApp::start_teach_trajectory);
	});
	
	// 默认两组工艺参数
	procData = std::vector<TrajectoryConfig<float>>(2);
	procData[0].speed = 10;
	procData[1].speed = 20;
	// 新增工艺
	QObject::connect(procedureWindow->ui->pushButton_3, &QPushButton::pressed, this, [&]() {
		int idx = procedureWindow->ui->tabWidget->count();
		procData.push_back(TrajectoryConfig<float>());
	});


	return 0;
}

int FSAIApp::init_menu_module() {
	// 打开工艺设置界面
	QObject::connect(mainWindow->ui->actionConfigure, &QAction::triggered, this, [&]() {
		// 更新工艺参数
		load_procedure_param(procedureWindow->ui->tabWidget->currentIndex());
		// 打开工艺设置页面
		procedureWindow->show();
		procedureWindow->activateWindow();
	});
	// 显示工艺
	QObject::connect(procedureWindow->ui->tabWidget, &QTabWidget::currentChanged, this, &FSAIApp::load_procedure_param);
	// 保存工艺
	QObject::connect(procedureWindow->ui->buttonBox, &QDialogButtonBox::accepted, this, &FSAIApp::save_current_procedure_param);

	// 打开轴号设置界面
	QObject::connect(mainWindow->ui->actionAxis_Configure, &QAction::triggered, this, [&]() {
		load_advance_config();
		// 打开轴号设置界面
		advanceConfigWindow->show();
		advanceConfigWindow->activateWindow();
	});
	// 保存设置
	QObject::connect(advanceConfigWindow->ui->buttonBox, &QDialogButtonBox::accepted, this, [&]() {
		save_advance_config();
		load_advance_config();
	});
	return 0;
}

int FSAIApp::init_monitor_module() {
	return 0;
}

int FSAIApp::init_log_module() {
	return 0;
}

int FSAIApp::init_control_module() {
	// 切换自动/手动
	QObject::connect(mainWindow->ui->checkBox_2, &QCheckBox::stateChanged, this, [&](int status) {
		if (!robotStatus->online) return;

		// 手动->自动
		if (status == 2) {
			// 关节速度切换到最大
			robot->set_manual_SpeedRatio(1);
			// 加减速调大
			robot->set_acceleration_time(0.2);
		}
		// 自动->手动
		else {
			robot->set_auto_SpeedRatio(1);
			// 加减速调小
			robot->set_acceleration_time(0.5);
		}
		update_speedRatio();
	});

	QObject::connect(mainWindow->ui->spinBox, &QSpinBox::editingFinished, this, [&]() {
		// 防止重复触发
		if ((!mainWindow->ui->spinBox->hasFocus()) && (mainWindow->ui->horizontalSlider->value() == mainWindow->ui->spinBox->value())) {
			return;
		}

		// 滑块同步速度
		mainWindow->ui->horizontalSlider->setValue(mainWindow->ui->spinBox->value());

		update_speedRatio();
	});

	QObject::connect(mainWindow->ui->horizontalSlider, &QSlider::sliderReleased, this, &FSAIApp::update_speedRatio);

	return 0;
}

void FSAIApp::update_speedRatio() {
	if (!robotStatus->online) return;

	// 自动模式
	if (mainWindow->ui->checkBox_2->isChecked()) {
		robot->set_auto_SpeedRatio(static_cast<float>(mainWindow->ui->spinBox->value()) / 100);
	}
	// 手动模式
	else {
		robot->set_manual_SpeedRatio(static_cast<float>(mainWindow->ui->spinBox->value()) / 100);
	}
}


// 连接机器人
void FSAIApp::switch_online() {
	if (robotStatus->online) {
		disconnect_robot();
	}
	else {
		connect_robot();
	}
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
	zScanner->resume();

	switch_auto(false);
	mainWindow->refresh_display_switch_online(true);
	update_speedRatio();

	return 0;
}

int FSAIApp::disconnect_robot() {
	mainWindow->ui->textBrowser->append("Disconnect");
	robotStatus->online = false;

	int ret = robot->disconnect();
	if (ret != 0) {
		mainWindow->ui->textBrowser->append("<Zmotion>: connect error " + QString::number(ret));
		robotStatus->online = true;
		return ret;
	}

	// 断开成功
	switch_auto(false);
	mainWindow->refresh_display_switch_online(false);

	return 0;
}

void FSAIApp::switch_auto(bool autoMode) {
	mainWindow->ui->checkBox_2->setChecked(autoMode);
	// 可能不会触发状态转换
	mainWindow->refresh_display_switch_auto(autoMode);
}

// 面板数据更新
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

void FSAIApp::load_procedure_param(int idx) {
	// 解析工艺参数
	Weave waveCfg;
	Arc_WeldingParaItem weldCfg;
	Track trackCfg;
	read_weld_param(procData[idx].appendix, waveCfg, weldCfg, trackCfg);

	// 速度
	procedureWindow->ui->doubleSpinBox_6->setValue(procData[idx].speed);

	// 启用摆焊
	procedureWindow->ui->groupBox_7->setChecked(waveCfg.Id > 0);
	procedureWindow->ui->comboBox_3->setCurrentIndex(waveCfg.Shape);
	procedureWindow->ui->doubleSpinBox_7->setValue(waveCfg.Freq);
	procedureWindow->ui->doubleSpinBox_8->setValue(waveCfg.LeftWidth);
	procedureWindow->ui->doubleSpinBox_9->setValue(waveCfg.RightWidth);
	procedureWindow->ui->spinBox_4->setValue(waveCfg.Dwell_left);
	procedureWindow->ui->spinBox_5->setValue(waveCfg.Dwell_right);
	procedureWindow->ui->radioButton_2->setChecked(waveCfg.Dwell_type == 0);
	procedureWindow->ui->radioButton->setChecked(waveCfg.Dwell_type == 1);
}

void FSAIApp::save_current_procedure_param() {
	int idx = procedureWindow->ui->tabWidget->currentIndex();
	// 解析工艺参数
	Weave waveCfg;
	Arc_WeldingParaItem weldCfg;
	Track trackCfg;

	// 速度
	procData[idx].speed = procedureWindow->ui->doubleSpinBox_6->value();

	// 启用摆焊
	waveCfg.Id = procedureWindow->ui->groupBox_7->isChecked() ? 1 : 0;
	waveCfg.Shape = procedureWindow->ui->comboBox_3->currentIndex();
	waveCfg.Freq = procedureWindow->ui->doubleSpinBox_7->value();
	waveCfg.LeftWidth = procedureWindow->ui->doubleSpinBox_8->value();
	waveCfg.RightWidth = procedureWindow->ui->doubleSpinBox_9->value();
	waveCfg.Dwell_left = procedureWindow->ui->spinBox_4->value();
	waveCfg.Dwell_right = procedureWindow->ui->spinBox_5->value();
	waveCfg.Dwell_type = procedureWindow->ui->radioButton_2->isChecked() ? 0 : 1;

	procData[idx].set_appendix(serialize_weld_param(waveCfg, weldCfg, trackCfg));
}


void FSAIApp::load_advance_config() {
	// 读取当前轴号
	std::vector<std::vector<int>> axisList = { robot->jointAxisIdx, robot->appAxisIdx, robot->tcpPosAxisIdx, 
		robot->tcpAngleAxisIdx,robot->camAxisIdx, robot->swingAxisIdx, robot->excuteAxis };
	std::vector<QComboBox*> combobox = { advanceConfigWindow->ui->comboBox , advanceConfigWindow->ui->comboBox_2,
		advanceConfigWindow->ui->comboBox_3, advanceConfigWindow->ui->comboBox_4, advanceConfigWindow->ui->comboBox_5,
		advanceConfigWindow->ui->comboBox_6, advanceConfigWindow->ui->comboBox_7 };

	for (size_t i = 0; i < axisList.size(); ++i) {
		QString str = QString::number(axisList[i][0]);
		for (size_t j = 1; j < axisList[i].size(); ++j) {
			str += ", " + QString::number(axisList[i][j]);
		}

		// 针对addItem方法可避免重复添加
		if (combobox[i]->findText(str) == -1) {
			if (combobox[i]->count() == 4) {
				//先进先出
				combobox[i]->removeItem(0);
			}
			combobox[i]->addItem(str);
		}
	}

}

void FSAIApp::save_advance_config() {
	// Ref: https://stackoverflow.com/a/55301228
	std::vector<std::reference_wrapper<std::vector<int>>> axisList = { robot->jointAxisIdx, robot->appAxisIdx, 
		robot->tcpPosAxisIdx, robot->tcpAngleAxisIdx,robot->camAxisIdx, robot->swingAxisIdx, robot->excuteAxis };
	std::vector<QComboBox*> combobox = { advanceConfigWindow->ui->comboBox , advanceConfigWindow->ui->comboBox_2,
		advanceConfigWindow->ui->comboBox_3, advanceConfigWindow->ui->comboBox_4, advanceConfigWindow->ui->comboBox_5,
		advanceConfigWindow->ui->comboBox_6, advanceConfigWindow->ui->comboBox_7 };


	for (size_t i = 0; i < axisList.size(); ++i) {
		std::vector<int> axis;
		QString str = combobox[i]->currentText();
		QStringList strList = str.split(",");

		for (auto& word : strList) {
			axis.push_back(word.toInt());
		}
		axisList[i].get() = axis;
		//axisList[i]->assign(axis.begin(), axis.end());
	}


}


// 读取示教点位置
std::vector<float> FSAIApp::read_teach_point(int row, const std::vector<float>& idxList) const  {
	std::vector<float> ans;

	for (auto& idx : idxList) {
		QString str = mainWindow->ui->tableWidget->item(row, idx)->text();
		QStringList strList = str.split(",");

		for (auto& word : strList) {
			ans.push_back(word.toFloat());
		}
	}
	return ans;
}

void FSAIApp::excute_selected_teach_point() {
	int row = mainWindow->ui->tableWidget->currentRow();
	if (row < 0) {
		mainWindow->ui->textBrowser->append("No teach point specified.");
		return;
	}

	// 工艺参数
	QWidget* item = mainWindow->ui->tableWidget->cellWidget(row, 2);
	int procIdx = ((QSpinBox*)item)->value();
	// 轨迹参数
	TrajectoryConfig<float> trajInfo = procData[procIdx];

	// 获取当前点的运动类型
	item = mainWindow->ui->tableWidget->cellWidget(row, 1);
	int mode = ((QComboBox*)item)->currentIndex();

	if (mode == 0) {
		// 切换到正解模式
		if (robot->forward_kinematics(10) != 0) {
			return;
		}

		std::vector<float> dpos = read_teach_point(row, { 4,5 });
		robot->moveJ_abs(dpos, trajInfo.speed / 100, false);
	}
	else {
		// 切换到逆解模式
		if (robot->inverse_kinematics(10) != 0) {
			return;
		}

		// 设置起点位置
		DiscreteTrajectory<float> traj;
		traj.set_starting_point(robotStatus->cPos);

		// 直线指令
		if (mode == 1) {
			std::vector<float> dpos = read_teach_point(row, { 3,5 });
			traj.add_line(dpos, trajInfo);
		}
		// 圆弧指令: 中间点
		else if (mode == 2) {
			// 当前点是最后点
			if (row + 1 >= mainWindow->ui->tableWidget->rowCount()) {
				mainWindow->ui->textBrowser->append("Error: moveC no end point.");
				return;
			}
			std::vector<float> dpos = read_teach_point(row + 1, { 3,5 });
			std::vector<float> midpos = read_teach_point(row, { 3,5 });
			traj.add_arc(dpos, midpos, trajInfo);
		}
		// 圆弧指令：终点
		else if (mode == -1) {
			// 当前点是第一点
			if (row < 1) {
				mainWindow->ui->textBrowser->append("Error: moveC no mid point.");
				return;
			}
			std::vector<float> dpos = read_teach_point(row, { 3,5 });
			std::vector<float> midpos = read_teach_point(row - 1, { 3,5 });
			traj.add_arc(dpos, midpos, trajInfo);
		}

		// 解析工艺参数
		Weave waveCfg;
		Arc_WeldingParaItem weldCfg;
		Track trackCfg;
		read_weld_param(trajInfo.appendix, waveCfg, weldCfg, trackCfg);

		// 执行轨迹
		if (waveCfg.Id > 0) {
			robot->swing_trajectory(traj);
		}
		else {
			robot->execute_discrete_trajectory(traj, false);
		}
	}
}

// todo 在子线程中执行
void FSAIApp::start_teach_trajectory()  const {
	int rowCount = mainWindow->ui->tableWidget->rowCount();
	if (rowCount < 1) {
		mainWindow->ui->textBrowser->append("No teach point specified.");
		return;
	}

	// 空间轨迹
	DiscreteTrajectory<float> traj;

	// 第一个点的运动类型
	QWidget* item = mainWindow->ui->tableWidget->cellWidget(0, 1);
	// 执行第一个点
	bool isFk = ((QComboBox*)item)->currentIndex() > 0 ? false : true;
	if (((QComboBox*)item)->currentIndex() > 0) {
		int ret = robot->inverse_kinematics(10);
		if (ret != 0) return;
		traj.set_starting_point(robotStatus->cPos);
	}
	else {
		int ret = robot->forward_kinematics(10);
		if (ret != 0) return;
	}

	for (int row = 0; row < rowCount; ++row) {
		// 获取当前点的运动类型
		item = mainWindow->ui->tableWidget->cellWidget(row, 1);
		int mode = ((QComboBox*)item)->currentIndex();

		// 工艺参数
		item = mainWindow->ui->tableWidget->cellWidget(row, 2);
		int procIdx = ((QSpinBox*)item)->value();
		// 轨迹参数
		TrajectoryConfig<float> trajInfo = procData[procIdx];

		if (isFk) {
			if (mode == 0) {
				std::vector<float> dpos = read_teach_point(row, { 4,5 });
				robot->moveJ_abs(dpos, trajInfo.speed / 100, false);
			}
			else {
				// 等待正解运动完成，并切换到逆解
				robot->wait_idle(robot->jointAxisIdx[0]);
				robot->inverse_kinematics(10);
				isFk = false;
				// 清空轨迹，设置起点
				traj.set_starting_point(robotStatus->cPos);

				// moveL
				if (mode == 1) {
					std::vector<float> dpos = read_teach_point(row, { 3,5 });
					traj.add_line(dpos, trajInfo);
				}
				// moveC2: 跳过moveC1
				else if (mode == -1) {
					std::vector<float> dpos = read_teach_point(row, { 3,5 });
					std::vector<float> midpos = read_teach_point(row - 1, { 3,5 });
					traj.add_arc(dpos, midpos, trajInfo);
				}
			}
		}
		else {
			if (mode == 0) {
				// 执行逆解
				robot->swing_trajectory(traj);

				// 等待逆解完成，切换到正解
				robot->wait_idle(robot->swingAxisIdx[0]);
				robot->forward_kinematics(10);
				isFk = true;

				std::vector<float> dpos = read_teach_point(row, { 4,5 });
				robot->moveJ_abs(dpos, trajInfo.speed / 100, false);
			}
			// moveL
			if (mode == 1) {
				std::vector<float> dpos = read_teach_point(row, { 3,5 });
				traj.add_line(dpos, trajInfo);
			}
			// moveC2: 跳过moveC1
			else if (mode == -1) {
				std::vector<float> dpos = read_teach_point(row, { 3,5 });
				std::vector<float> midpos = read_teach_point(row - 1, { 3,5 });
				traj.add_arc(dpos, midpos, trajInfo);
			}
		}
	}

	// 执行最后的空间轨迹
	if (!isFk) {
		robot->swing_trajectory(traj);
	}
}