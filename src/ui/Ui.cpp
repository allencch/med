#include <cstdio>
#include <iostream>

#include <QtUiTools>
#include <QtDebug>

#include "med/MedException.hpp"
#include "ui/Ui.hpp"
#include "ui/ProcessEventListener.hpp"
#include "ui/ComboBoxDelegate.hpp"

using namespace std;

MedUi::MedUi(QApplication* app) {
  this->app = app;
  med = new MemEd();

  loadUiFiles();
  loadProcessUi();
  setupStatusBar();
  setupScanTreeView();
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
  scanTreeView = mainWindow->findChild<QTreeView*>("scanTreeView");
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

void MedUi::setupScanTreeView() {
  scanModel = new TreeModel(this, mainWindow);
  scanTreeView->setModel(scanModel);
  ComboBoxDelegate* delegate = new ComboBoxDelegate();
  scanTreeView->setItemDelegateForColumn(SCAN_COL_TYPE, delegate);
  QObject::connect(scanTreeView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(onScanTreeViewClicked(QModelIndex)));
  QObject::connect(scanTreeView,
                   SIGNAL(doubleClicked(QModelIndex)),
                   this,
                   SLOT(onScanTreeViewDoubleClicked(QModelIndex)));

  QObject::connect(scanModel,
                   SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                   this,
                   SLOT(onScanTreeViewDataChanged(QModelIndex, QModelIndex, QVector<int>)));
  scanTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

  scanModel->clearAll();
  if(med->getScans().size() <= SCAN_ADDRESS_VISIBLE_SIZE) {
    scanModel->addScan(scanType);
  }

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

  if(med->getScans().size() <= SCAN_ADDRESS_VISIBLE_SIZE) {
    scanModel->addScan(scanType);
  }

  updateNumberOfAddresses();
}

void MedUi::onScanTreeViewClicked(const QModelIndex &index) {
  if(index.column() == SCAN_COL_TYPE) {
    scanTreeView->edit(index); //Trigger edit by 1 click
  }
}

void MedUi::onScanTreeViewDoubleClicked(const QModelIndex &index) {
  if (index.column() == SCAN_COL_VALUE) {
    // TODO: lock state
  }
}

void MedUi::onScanTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
  // qDebug() << topLeft << bottomRight << roles;
  if (topLeft.column() == SCAN_COL_VALUE) {
    // TODO: unlock state
    // tryUnlock(scanUpdateMutex);
    // scanState = UiState::Idle;
  }
}

void MedUi::updateNumberOfAddresses() {
  char message[128];
  sprintf(message, "%ld", med->getScans().size());
  mainWindow->findChild<QLabel*>("found")->setText(message);
}
