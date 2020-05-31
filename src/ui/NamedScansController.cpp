#include <iostream>
#include <QtUiTools>
#include "ui/Ui.hpp"
#include "ui/NamedScansController.hpp"
#include "mem/StringUtil.hpp"

using namespace std;

NamedScansController::NamedScansController(MedUi *mainUi) {
  this->mainUi = mainUi;

  mainWindow = mainUi->mainWindow;
  med = mainUi->med;
  namedScans = &(med->getNamedScans());

  comboBox = mainWindow->findChild<QComboBox*>("namedScans");

  QObject::connect(mainWindow->findChild<QWidget*>("namedScan_add"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onAddClicked()));
  QObject::connect(mainWindow->findChild<QWidget*>("namedScan_delete"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onDeleteClicked()));
  QObject::connect(comboBox,
                   SIGNAL(currentIndexChanged(int)),
                   this,
                   SLOT(onComboBoxChanged(int)));
}

void NamedScansController::onAddClicked() {
  auto nameInput = mainWindow->findChild<QLineEdit*>("namedScan_name");
  string name = nameInput->text().toStdString();
  auto trimmed = StringUtil::trim(name);
  auto result = namedScans->addNewScan(trimmed);
  if (!result) return;

  comboBox->addItem(QString(trimmed.c_str()));
  nameInput->setText("");

  selectByName(trimmed);
}

void NamedScansController::selectByName(string name) {
  auto trimmed = StringUtil::trim(name);
  int index = comboBox->findText(QString(trimmed.c_str()));
  if (index < 0) return;

  comboBox->setCurrentIndex(index);
  namedScans->setActiveName(trimmed);
}

void NamedScansController::onDeleteClicked() {
  string name = comboBox->currentText().toStdString();
  auto trimmed = StringUtil::trim(name);
  int index = comboBox->currentIndex();
  auto result = namedScans->remove(trimmed);
  if (!result) return;

  comboBox->setCurrentIndex(0); // Default
  comboBox->removeItem(index);
}

void NamedScansController::onComboBoxChanged(int) {
  string name = comboBox->currentText().toStdString();
  auto trimmed = StringUtil::trim(name);
  namedScans->setActiveName(trimmed);
  updateScanTree();
  updateScanType();
}

void NamedScansController::updateScanTree() {
  auto count = namedScans->getMemList()->size();
  if (count > SCAN_ADDRESS_VISIBLE_SIZE) return;

  mainUi->scanUpdateMutex->lock();
  mainUi->scanModel->addScan(namedScans->getScanType());
  mainUi->scanUpdateMutex->unlock();
  mainUi->updateNumberOfAddresses();
}

void NamedScansController::updateScanType() {
  auto scanTypeCombo = mainWindow->findChild<QComboBox*>("scanType");
  auto scanType = QString(namedScans->getScanType().c_str());
  int index = scanTypeCombo->findText(scanType);
  if (index < 0) return;

  scanTypeCombo->setCurrentIndex(index);
}
