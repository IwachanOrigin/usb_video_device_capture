
#include <QDebug>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_currentDeviceIndex(-1)
{
  ui.setupUi(this);

  m_vecDevNames.clear();
  m_devices.getDeviceNameList(m_vecDevNames);
  if (!m_vecDevNames.empty())
  {
    for (auto item : m_vecDevNames)
    {
      QString qsItem = QString::fromStdWString(item.deviceName);
      ui.comboBoxCameras->addItem(qsItem);
    }
    m_currentDeviceIndex = 0;
  }

  if (m_currentDeviceIndex > -1)
  {
    //
    m_vecMediaInfo.clear();
    m_devices.setCurrentVideoDeviceIndex(m_currentDeviceIndex);
    m_devices.getVideoDeviceMediaList(m_vecMediaInfo);
    if (!m_vecMediaInfo.empty())
    {
      for (auto item : m_vecMediaInfo)
      {
        QString fmtName = QString::fromStdWString(item.formatSubtypeName);
        QString qsItem = QString("%1x%2, %3, %4, %5").arg(item.width).arg(item.height).arg(fmtName).arg(item.stride).arg(item.samplesize);
        ui.comboBoxFormats->addItem(qsItem);
      }
    }
  }
}

void MainWindow::closeEvent(QCloseEvent* e)
{
}

