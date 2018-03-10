#include <cstdio>
#include <iostream>

#include <QtUiTools>
#include <QtDebug>

#include "med/MedException.hpp"
#include "ui/Ui.hpp"
#include "ui/ProcessEventListener.hpp"

using namespace std;

MedUi::MedUi(QApplication* app) {
  this->app = app;
  med = new MemEd();

  loadUiFiles();
  loadProcessUi();
  setupStatusBar();
  setupSignals();
  setupUi();
  // TODO: other action here
}

MedUi::~MedUi() {
  delete med;
}

void MedUi::loadUiFiles() {
  QUiLoader loader;
  QFile file("./main-qt.ui");
  file.open(QFile::ReadOnly);
  mainWindow = loader.load(&file);
  file.close();

  selectedProcessLine = mainWindow->findChild<QLineEdit*>("selectedProcess");
  scanTypeCombo = mainWindow->findChild<QComboBox*>("scanType");
  // TODO: other setup here
}

void MedUi::loadProcessUi() {
  QUiLoader loader;
  //Cannot put the followings to another method
  processDialog = new QDialog(mainWindow); //If put this to another method, then I cannot set the mainWindow as the parent
  QFile processFile("./process.ui");
  processFile.open(QFile::ReadOnly);

  processSelector = loader.load(&processFile, processDialog);
  processFile.close();

  processTreeWidget = processSelector->findChild<QTreeWidget*>("processTreeWidget");

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(processSelector);
  processDialog->setLayout(layout);
  processDialog->setModal(true);
  processDialog->resize(400, 400);

  //Add signal
  QObject::connect(processTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onProcessItemDblClicked(QTreeWidgetItem*, int)));

  processTreeWidget->installEventFilter(new ProcessDialogEventListener(this));
}

void MedUi::setupUi() {
  scanTypeCombo->setCurrentIndex(1);
  mainWindow->show();
}

void MedUi::setupStatusBar() {
  //Statusbar message
  statusBar = mainWindow->findChild<QStatusBar*>("statusbar");
  statusBar->showMessage("Tips: Left panel is scanned address. Right panel is stored address.");
}

void MedUi::setupSignals() {
  QObject::connect(mainWindow->findChild<QWidget*>("process"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onProcessClicked()));

  QObject::connect(mainWindow->findChild<QWidget*>("scanButton"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onScanClicked()));

  QObject::connect(mainWindow->findChild<QWidget*>("filterButton"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onFilterClicked()));
}

void MedUi::onProcessClicked() {
  med->listProcesses();
  processDialog->show();
  processTreeWidget->clear();

  for (int i = med->processes.size() - 1; i >= 0; i--) {
    QTreeWidgetItem* item = new QTreeWidgetItem(processTreeWidget);
    item->setText(0, med->processes[i].pid.c_str());
    item->setText(1, med->processes[i].cmdline.c_str());
  }
}

void MedUi::onProcessItemDblClicked(QTreeWidgetItem* item, int) {
  int index = item->treeWidget()->indexOfTopLevelItem(item); //Get the current row index

  Process process = med->selectProcessByIndex(med->processes.size() - 1 - index);

  selectedProcessLine->setText(QString::fromLatin1((process.pid + " " + process.cmdline).c_str())); //Do not use fromStdString(), it will append with some unknown characters

  processDialog->hide();
}

void MedUi::onScanClicked() {
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }

  string scanValue = mainWindow->findChild<QLineEdit*>("scanEntry")->text().toStdString();
  if (QString(scanValue.c_str()).trimmed() == "") {
    return;
  }

  string scanType = scanTypeCombo->currentText().toStdString();
  // TODO: Encoding manager here

  try {
    med->scan(scanValue, scanType);
  } catch(MedException &ex) {
    cerr << "scan: "<< ex.what() <<endl;
  }

  // TODO: Update the scanned

  if (QString(scanValue.c_str()).trimmed() == "?") {
    statusBar->showMessage("Snapshot saved");
  }
  else {
    updateNumberOfAddresses();
  }
}


void MedUi::onFilterClicked() {
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }

  string scanValue = mainWindow->findChild<QLineEdit*>("scanEntry")->text().toStdString();
  if (QString(scanValue.c_str()).trimmed() == "") {
    return;
  }

  string scanType = scanTypeCombo->currentText().toStdString();

  med->filter(scanValue, scanType);

  updateNumberOfAddresses();
}

void MedUi::updateNumberOfAddresses() {
  char message[128];
  sprintf(message, "%ld", med->getMems().size());
  mainWindow->findChild<QLabel*>("found")->setText(message);
}
