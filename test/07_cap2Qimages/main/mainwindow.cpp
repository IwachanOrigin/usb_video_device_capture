
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);

  m_devices.getDeviceNameList(m_vecDevNames);
  if (!m_vecDevNames.empty())
  {
    
  }
}

void MainWindow::closeEvent(QCloseEvent* e)
{
}

