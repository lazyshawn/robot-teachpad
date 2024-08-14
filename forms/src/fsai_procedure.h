#pragma once

#include <QDialog>
#include "procedure.h"

#include <memory>

class ProcedureWindow : public QDialog {
public:
  // 主页面
  std::shared_ptr<Ui_Procedure> uiProcedure;

private:


public:
  ProcedureWindow();
  virtual ~ProcedureWindow();

private:

  Q_OBJECT
public slots:
  // void switch_connection();
  // void jog_moving(int axis, int type);


};

