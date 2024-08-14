
#include "fsai_mainwindow.h"

#include <memory>


MainWindow::MainWindow() {
	ui = std::make_shared<Ui_MainWindow>();
	
	ui->setupUi(this);

	// 初始化连接模块
	init_connect_module();
	// 初始化点动模块
	init_jog_module();
	// 初始化示教模块
	init_teach_module();
	// 初始化菜单栏
	init_menu_module();
	// 初始化监控页面
	init_monitor_module();
	// 初始化日志页面
	init_log_module();

	// 禁用控制模块
	// uiMainWindow->groupBox->setEnabled(false);
	// uiMainWindow->groupBox_2->setEnabled(false);
}


MainWindow::~MainWindow(){
}


int MainWindow::init_log_module() {
	QObject::connect(ui->pushButton_9, &QPushButton::pressed, ui->textBrowser, &QTextBrowser::clear);

	QObject::connect(ui->lineEdit, &QLineEdit::editingFinished, this, [&](){
		ui->textBrowser->append(ui->lineEdit->text());
		ui->lineEdit->selectAll();
	});

	return 0;
}

void MainWindow::jog_moving(int axis, int type) {
	/*
	// 运动结束
	if (type == 0) {
		ui->textBrowser->append("Jog moving end");
	}
	// 正向运动
	else if (type > 0){
		QString msg = "(" + QString::fromStdString(get_jog_move_type_str()) + "): " + QString::fromStdString(jointName[std::abs(type)-1][axis]) + " +";
		ui->textBrowser->append(msg);
	}
	// 负向运动
	else {
		QString msg = "(" + QString::fromStdString(get_jog_move_type_str()) + "): " + QString::fromStdString(jointName[std::abs(type)-1][axis]) + " -";
		ui->textBrowser->append(msg);
	}*/
	return;
}


int MainWindow::init_connect_module() {
	ui->comboBox->addItem("127.0.0.1");
	ui->comboBox->addItem("192.168.0.11");
	ui->comboBox->addItem("169.182.1.11");
	ui->comboBox->addItem("0");

	// 切换连接
	// QObject::connect(uiMainWindow->ConnectMenu, &QPushButton::pressed, this, &MainWindow::switch_connection);

	//ui->comboBox->setFocus();

	return 0;
}


int MainWindow::init_jog_module() {
	jogMoveBtn = {ui->pushButton_12, ui->pushButton_13,
					ui->pushButton_14, ui->pushButton_15,
					ui->pushButton_16, ui->pushButton_17,
					ui->pushButton_18, ui->pushButton_19,
					ui->pushButton_20, ui->pushButton_21,
					ui->pushButton_22, ui->pushButton_23,
					ui->pushButton_24, ui->pushButton_25,
					ui->pushButton_26, ui->pushButton_27,
					ui->pushButton_28, ui->pushButton_29 };
	jogMoveLabel = {ui->label, ui->label_2, ui->label_3,
					ui->label_4, ui->label_5, ui->label_6,
					ui->label_7, ui->label_8, ui->label_9 };


	// ***************************** 点动轴号标签 ********************************
	ui->radioButton->setChecked(true);
	QObject::connect(ui->radioButton, &QRadioButton::toggled, this, [&](bool flag) {
		if (!flag) return;
		ui->textBrowser->append("Change jog move type to " + QString::fromStdString(jogMoveTypeStr[0]));
		for (size_t i = 0; i < 9; ++i) {
			jogMoveLabel[i]->setText(QString::fromStdString(jointName[0][i]));
		}
	});
	QObject::connect(ui->radioButton_2, &QRadioButton::toggled, this, [&](bool flag) {
		if (!flag) return;
		ui->textBrowser->append("Change jog move type to " + QString::fromStdString(jogMoveTypeStr[1]));
		for (size_t i = 0; i < 9; ++i) {
			jogMoveLabel[i]->setText(QString::fromStdString(jointName[1][i]));
		}
	});
	QObject::connect(ui->radioButton_3, &QRadioButton::toggled, this, [&](bool flag) {
		if (!flag) return;
		ui->textBrowser->append("Change jog move type to " + QString::fromStdString(jogMoveTypeStr[1]));
		for (size_t i = 0; i < 9; ++i) {
			jogMoveLabel[i]->setText(QString::fromStdString(jointName[1][i]));
		}
	});

  /*
  for (int i=0; i<9; ++i) {
    QObject::connect(jogMoveBtn[2*i], &QPushButton::pressed, this, [&, i](){ MainWindow::jog_moving(i,-1*(MainWindow::get_jog_move_type()+1)); });
    QObject::connect(jogMoveBtn[2*i], &QPushButton::released, this, [&, i](){ MainWindow::jog_moving(i,0); });

    QObject::connect(jogMoveBtn[2*i+1], &QPushButton::pressed, this, [&, i](){ MainWindow::jog_moving(i,MainWindow::get_jog_move_type()+1); });
    QObject::connect(jogMoveBtn[2*i+1], &QPushButton::released, this, [&, i](){ MainWindow::jog_moving(i,0); });
  }

  // ***************************** 点动轴号标签 ********************************
  QObject::connect(ui->radioButton, &QRadioButton::toggled, this, [&](bool flag){
      if (!flag) return;
      MainWindow::jogMoveType = 0;
      ui->textBrowser->append("Change jog move type to " + QString::fromStdString(MainWindow::get_jog_move_type_str()));
      for (size_t i=0; i<9; ++i) {
        jogMoveLabel[i]->setText(QString::fromStdString(jointName[jogMoveType][i]));
      }
  });
  QObject::connect(ui->radioButton_2, &QRadioButton::toggled, this, [&](bool flag){
      if (!flag) return;
      MainWindow::jogMoveType = 1;
      ui->textBrowser->append("Change jog move type to " + QString::fromStdString(MainWindow::get_jog_move_type_str()));
      for (size_t i=0; i<9; ++i) {
        jogMoveLabel[i]->setText(QString::fromStdString(jointName[jogMoveType][i]));
      }
  });
  QObject::connect(ui->radioButton_3, &QRadioButton::toggled, this, [&](bool flag){
      if (!flag) return;
      MainWindow::jogMoveType = 2;
      ui->textBrowser->append("Change jog move type to " + QString::fromStdString(MainWindow::get_jog_move_type_str()));
      for (size_t i=0; i<9; ++i) {
        jogMoveLabel[i]->setText(QString::fromStdString(jointName[jogMoveType][i]));
      }
  });
  */

  // ***************************** 速度设置 ********************************
  QObject::connect(ui->spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->horizontalSlider, &QSlider::setValue);
  QObject::connect(ui->horizontalSlider, &QSlider::valueChanged, this, [&](int value) {
      ui->spinBox->setValue(value);
      // 更新速度
      //speedRatio = ui->spinBox->value();
  });
  return 0;
}


int MainWindow::init_teach_module() {
	// 设置表头
	QStringList rowHeader;
	rowHeader << "Seq" << "Move" << "Procedure" << "TCP Pos" << "Joint Pos" << "Appendix Pos";
	ui->tableWidget->setColumnCount(rowHeader.count());
	ui->tableWidget->setHorizontalHeaderLabels(rowHeader);
	ui->tableWidget->verticalHeader()->setDefaultSectionSize(20);
	// 隐藏列表头
	ui->tableWidget->verticalHeader()->setVisible(false);
	// 交替色
	ui->tableWidget->setAlternatingRowColors(true);

	ui->tableWidget->setColumnWidth(0, 50);
	ui->tableWidget->setColumnWidth(1, 100);
	ui->tableWidget->setColumnWidth(2, 150);
	ui->tableWidget->setColumnWidth(3, 400);
	ui->tableWidget->setColumnWidth(4, 400);
	ui->tableWidget->setColumnWidth(5, 300);
	// uiMainWindow->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	// uiMainWindow->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	// uiMainWindow->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	// 单击选中行
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	
	QObject::connect(ui->pushButton_2, &QPushButton::pressed, this, &MainWindow::record_teach_point);
	QObject::connect(ui->pushButton_3, &QPushButton::pressed, this, &MainWindow::delete_teach_point);

	// QObject::connect(uiMainWindow->pushButton_8, &QPushButton::pressed, &procedureWindow, [&](){
	//     procedureWindow.show();
	//     procedureWindow.activateWindow();
	// });

  return 0;
}


int MainWindow::init_menu_module() {
	//QObject::connect(ui->actionClose, &QAction::triggered, this, &MainWindow::close);

	return 0;
}

int MainWindow::init_monitor_module() {
	// 设置表头
	QStringList rowHeader;
	rowHeader << "Seq" << "Joint" << "TCP";
	ui->tableWidget_2->setColumnCount(rowHeader.count());
	ui->tableWidget_2->setHorizontalHeaderLabels(rowHeader);
	ui->tableWidget_2->verticalHeader()->setDefaultSectionSize(20);
	// 隐藏列表头
	ui->tableWidget_2->verticalHeader()->setVisible(false);
	// 交替色
	ui->tableWidget_2->setAlternatingRowColors(true);
	ui->tableWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui->tableWidget_2->setMinimumHeight(380);
	ui->tableWidget_2->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	// 单击选中行
	ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);

	for (size_t i=0; i<9; ++i) {
		ui->tableWidget_2->insertRow(i);
		// 序号
		QTableWidgetItem* seqItem = new QTableWidgetItem(QString::number(i));
		ui->tableWidget_2->setItem(i, 0, seqItem);
	}

	return 0;
}


void MainWindow::record_teach_point() {
	int row = ui->tableWidget->currentRow();
	// 若未选中则在最后插入，选中则在下方插入
	row = row < 0 ? ui->tableWidget->rowCount() : row + 1;

	// 新增一行
	ui->tableWidget->insertRow(row);

	// 填充行内容
	// 序号
	QTableWidgetItem* seqItem = new QTableWidgetItem(QString::number(row));
	seqItem->setFlags(seqItem->flags() & (~Qt::ItemIsEditable));
	ui->tableWidget->setItem(row, 0, seqItem);
	// 轨迹类型
	QComboBox* typeComboBox = new QComboBox();
	typeComboBox->addItem("     J");
	typeComboBox->addItem("     L");
	typeComboBox->addItem("     C");
	ui->tableWidget->setCellWidget(row, 1, typeComboBox);
	// 如果上一条运动为圆弧中间点，则当前类型为圆弧终点
	if (row > 0) {
		QWidget* preItem = ui->tableWidget->cellWidget(row - 1, 1);
		if (((QComboBox*)preItem)->currentIndex() == 2) {
			typeComboBox->setCurrentIndex(-1);
		}
	}
	QObject::connect(typeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](int idx) {
		int row = ui->tableWidget->currentRow();
		// 下一条运动类型为圆弧终点
		if (idx == 2 && row+1 < ui->tableWidget->rowCount()) {
			QWidget* preItem = ui->tableWidget->cellWidget(row + 1, 1);
			((QComboBox*)preItem)->setCurrentIndex(-1);
		}
		// 如果上一条运动为圆弧中间点，则当前类型为圆弧终点
		if (row > 0) {
			QWidget* preItem = ui->tableWidget->cellWidget(row - 1, 1);
			if (((QComboBox*)preItem)->currentIndex() == 2) {
				QWidget* preItem = ui->tableWidget->cellWidget(row, 1);
				((QComboBox*)preItem)->setCurrentIndex(-1);
			}
		}
	});
	// 工艺号
	QSpinBox* procedureBox = new QSpinBox();
	procedureBox->setMaximum(10);
	procedureBox->setPrefix("Proc. ");
	procedureBox->setButtonSymbols(QSpinBox::NoButtons);
	procedureBox->setAlignment(Qt::AlignHCenter);
	ui->tableWidget->setCellWidget(row, 2, procedureBox);

	// 更新后续行的行号
	for (size_t i=row+1; i<ui->tableWidget->rowCount(); ++i) {
		ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
	}

	// 选中新插入的行
	ui->tableWidget->selectRow(row);
	// 将焦点设置在table上，可以 <C-A> 选中所有
	ui->tableWidget->setFocus();

	// 输出日志
}

void MainWindow::delete_teach_point() {
  // 被移除的行
  std::vector<int> rowIdx;

  // Ref: 删除所有选中行
  // https://developer.baidu.com/article/details/2827678
  // 获取选择模型
  QItemSelectionModel* selectionModel = ui->tableWidget->selectionModel();
  // 获取所有选中的行索引
  QModelIndexList selectedRows = selectionModel->selectedRows();
  for (auto & selected : selectedRows) {
    rowIdx.push_back(selected.row());
  }

  if (rowIdx.size() < 1) return;
  std::sort(rowIdx.begin(), rowIdx.end());

  // 从前往后
  for (size_t i=0; i<rowIdx.size(); ++i) {
    ui->tableWidget->removeRow(rowIdx[i]-i);
  }

  // 更新行索引
  for (size_t i=0; i<rowIdx.size(); ++i) {
    int beg = rowIdx[i], end = (i < rowIdx.size()-1) ? rowIdx[i+1] : ui->tableWidget->rowCount() + i;
    for (size_t j=beg; j<end; ++j) {
      ui->tableWidget->setItem(j-i, 0, new QTableWidgetItem(QString::number(j-i)));
    }
  }

  // 选中被删除的最后一行
  if (ui->tableWidget->rowCount() > 0) {
    size_t selected = rowIdx.back() - rowIdx.size() + 1;
    selected = selected >= ui->tableWidget->rowCount() ? ui->tableWidget->rowCount()-1 : selected;
    ui->tableWidget->selectRow(selected);
  }
}

