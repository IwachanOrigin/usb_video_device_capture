
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_mainwindow.h"


#include "devicesinfo.h"
#include "devicecommon.h"

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
  int m_currentFormatIndex;
  QTimer* m_updateTimer;

  void updateMediaInfo();
  void eventConnect();
  void eventDisconnect();

private slots:
  void slotChangedCamera();
  void slotChangedFormat();
  void slotCaptureStart();
  void slotCaptureStop();
  void slotUpdateTexture();
};

#endif // MAIN_WINDOW_H_
