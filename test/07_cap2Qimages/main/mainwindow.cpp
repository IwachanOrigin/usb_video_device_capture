
#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_currentDeviceIndex(-1)
  , m_currentFormatIndex(-1)
  , m_updateTimer(nullptr)
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
    this->slotChangedFormat();
  }
}

void MainWindow::eventConnect()
{
  QObject::connect(ui.comboBoxCameras, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::slotChangedCamera);
  QObject::connect(ui.comboBoxFormats, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::slotChangedFormat);
  QObject::connect(ui.pushButtonCapStart, &QPushButton::pressed, this, &MainWindow::slotCaptureStart);
  QObject::connect(ui.pushButtonCapStop, &QPushButton::pressed, this, &MainWindow::slotCaptureStop);
}

void MainWindow::eventDisconnect()
{
  QObject::disconnect(ui.comboBoxCameras, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::slotChangedCamera);
  QObject::disconnect(ui.comboBoxFormats, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::slotChangedFormat);
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

void MainWindow::slotChangedFormat()
{
  m_currentFormatIndex = ui.comboBoxFormats->currentIndex();
  m_devices.setCurrentVideoFormatIndex(m_currentFormatIndex);
}

void MainWindow::slotCaptureStart()
{
  m_devices.captureStart();
  m_updateTimer = new QTimer(this);
  QObject::connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::slotUpdateTexture);
  m_updateTimer->start(500);
}

void MainWindow::slotCaptureStop()
{
  m_devices.captureStop();
  m_updateTimer->stop();
  delete m_updateTimer;
  m_updateTimer = nullptr;
}

void MainWindow::slotUpdateTexture()
{
  unsigned char* buffer = nullptr;
  m_devices.updateImage(buffer);
  qDebug() << "cap : " << buffer;
  delete buffer;
}