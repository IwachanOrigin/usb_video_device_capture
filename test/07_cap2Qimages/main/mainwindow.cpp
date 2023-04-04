
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
    // update media info.
    this->updateMediaInfo();
  }
  this->eventConnect();
}

void MainWindow::closeEvent(QCloseEvent* e)
{
}

void MainWindow::updateMediaInfo()
{
  m_vecMediaInfo.clear();
  m_devices.setCurrentVideoDeviceIndex(m_currentDeviceIndex);
  m_devices.getVideoDeviceMediaList(m_vecMediaInfo);
  if (!m_vecMediaInfo.empty())
  {
    ui.comboBoxFormats->clear();
    for (auto item : m_vecMediaInfo)
    {
      QString fmtName = QString::fromStdWString(item.formatSubtypeName);
      QString qsItem = QString("%1x%2, %3, %4, %5").arg(item.width).arg(item.height).arg(fmtName).arg(item.stride).arg(item.samplesize);
      ui.comboBoxFormats->addItem(qsItem);
    }
  }
}

void MainWindow::eventConnect()
{
  QObject::connect(ui.comboBoxCameras, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::slotChangedCamera);
  QObject::connect(ui.pushButtonCapStart, &QPushButton::pressed, this, &MainWindow::slotCaptureStart);
  QObject::connect(ui.pushButtonCapStop, &QPushButton::pressed, this, &MainWindow::slotCaptureStop);
}

void MainWindow::eventDisconnect()
{
  QObject::disconnect(ui.comboBoxCameras, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::slotChangedCamera);
  QObject::disconnect(ui.pushButtonCapStart, &QPushButton::pressed, this, &MainWindow::slotCaptureStart);
  QObject::disconnect(ui.pushButtonCapStop, &QPushButton::pressed, this, &MainWindow::slotCaptureStop);
}

void MainWindow::slotChangedCamera()
{
  this->eventDisconnect();
  m_currentDeviceIndex = ui.comboBoxCameras->currentIndex();
  this->updateMediaInfo();
  this->eventConnect();
}

void MainWindow::slotCaptureStart()
{
}

void MainWindow::slotCaptureStop()
{
}
