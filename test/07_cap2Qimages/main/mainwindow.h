
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

#include "devicecommon.h"
#include "devicesinfo.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = Q_NULLPTR);

protected:
  void closeEvent(QCloseEvent* e);

private:
  Ui::MainWindow ui;
  DevicesInfo m_devices;
  std::vector<DeviceInfo> m_vecDevNames;
  std::vector<DeviceMediaInfo> m_vecMediaInfo;
  int m_currentDeviceIndex;
};

#endif // MAIN_WINDOW_H_
