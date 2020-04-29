#include <cstdio>
#include <iostream>

#include <QtUiTools>
#include <QtDebug>

#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "ui/Ui.hpp"
#include "ui/ProcessEventListener.hpp"
#include "ui/ScanTreeEventListener.hpp"
#include "ui/StoreTreeEventListener.hpp"
#include "ui/ComboBoxDelegate.hpp"
#include "ui/CheckBoxDelegate.hpp"
#include "ui/EncodingManager.hpp"
#include "ui/MemEditor.hpp"

using namespace std;

MedUi::MedUi(QApplication* app) {
  this->app = app;
  this->autoRefresh = true;
  this->fastScan = true;
  med = new MemEd();
  scanUpdateMutex = &med->getScanListMutex();

  loadUiFiles();
  loadProcessUi();
  loadMemEditor();
  setupStatusBar();
  setupScanTreeView();
  setupStoreTreeView();
  setupSignals();
  setupUi();

  encodingManager = new EncodingManager(this);
  setScanState(UiState::Idle);
  setStoreState(UiState::Idle);
}

MedUi::~MedUi() {
  delete med;
  delete encodingManager;

  refreshThread->join();
  delete refreshThread;
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
  storeTreeView = mainWindow->findChild<QTreeView*>("storeTreeView");

  scanTreeView->installEventFilter(new ScanTreeEventListener(scanTreeView, this));
  storeTreeView->installEventFilter(new StoreTreeEventListener(storeTreeView, this));

  notesArea = mainWindow->findChild<QPlainTextEdit*>("notes");
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

void MedUi::loadMemEditor() {
  memEditor = new MemEditor(this);
}

void MedUi::setupUi() {
  scanTypeCombo->setCurrentIndex(2); // int32
  mainWindow->show();
  qRegisterMetaType<QVector<int>>(); // For multithreading

  refreshThread = new std::thread(MedUi::refresh, this);

  QAction* showNotesAction = mainWindow->findChild<QAction*>("actionShowNotes");
  if (showNotesAction->isChecked()) {
    notesArea->show();
  }
  else {
    notesArea->hide();
  }
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

  QObject::connect(mainWindow->findChild<QWidget*>("pauseCheckbox"),
                   SIGNAL(clicked(bool)),
                   this,
                   SLOT(onPauseCheckboxClicked(bool)));

  QObject::connect(mainWindow->findChild<QPushButton*>("scanAdd"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onScanAddClicked()));
  QObject::connect(mainWindow->findChild<QPushButton*>("scanAddAll"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onScanAddAllClicked()));
  QObject::connect(mainWindow->findChild<QWidget*>("scanClear"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onScanClearClicked()));

  QObject::connect(mainWindow->findChild<QAction*>("actionSaveAs"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onSaveAsTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionOpen"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onOpenTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionSave"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onSaveTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionQuit"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onQuitTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionReload"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onReloadTriggered()));

  QObject::connect(mainWindow->findChild<QAction*>("actionShowNotes"),
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(onShowNotesTriggered(bool)));
  QObject::connect(notesArea,
                   SIGNAL(textChanged()),
                   this,
                   SLOT(onNotesAreaChanged()));
  QObject::connect(mainWindow->findChild<QAction*>("actionAutoRefresh"),
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(onAutoRefreshTriggered(bool)));
  QObject::connect(mainWindow->findChild<QAction*>("actionRefresh"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onRefreshTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionResumeProcess"),
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(onResumeProcessTriggered(bool)));
  QObject::connect(mainWindow->findChild<QAction*>("actionFastScan"),
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(onFastScanTriggered(bool)));

  QObject::connect(mainWindow->findChild<QPushButton*>("storeClear"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onStoreClearClicked()));
  QObject::connect(mainWindow->findChild<QPushButton*>("nextAddress"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onStoreNextClicked()));
  QObject::connect(mainWindow->findChild<QPushButton*>("prevAddress"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onStorePrevClicked()));
  QObject::connect(mainWindow->findChild<QPushButton*>("storeShift"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onStoreShiftClicked()));
  QObject::connect(mainWindow->findChild<QPushButton*>("storeUnshift"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onStoreUnshiftClicked()));
  QObject::connect(mainWindow->findChild<QPushButton*>("moveAddress"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onStoreMoveClicked()));
  QObject::connect(mainWindow->findChild<QAction*>("actionNewAddress"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onNewAddressTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionDeleteAddress"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onDeleteAddressTriggered()));
  QObject::connect(mainWindow->findChild<QAction*>("actionMemEditor"),
                   SIGNAL(triggered()),
                   this,
                   SLOT(onMemEditorTriggered()));

  QObject::connect(mainWindow->findChild<QLineEdit*>("scopeStart"),
                   SIGNAL(editingFinished()),
                   this,
                   SLOT(onScopeStartEdited()));
  QObject::connect(mainWindow->findChild<QLineEdit*>("scopeEnd"),
                   SIGNAL(editingFinished()),
                   this,
                   SLOT(onScopeEndEdited()));
}

void MedUi::setupScanTreeView() {
  scanModel = new TreeModel(this, mainWindow);
  scanTreeView->setModel(scanModel);
  scanTreeView->setColumnWidth(SCAN_COL_TYPE, 90);
  scanTreeView->setUniformRowHeights(true);
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

void MedUi::setupStoreTreeView() {
  storeModel = new StoreTreeModel(this, mainWindow);
  storeTreeView->setModel(storeModel);
  storeTreeView->setColumnWidth(STORE_COL_TYPE, 90);
  storeTreeView->setUniformRowHeights(true);
  ComboBoxDelegate* storeDelegate = new ComboBoxDelegate();
  storeTreeView->setItemDelegateForColumn(STORE_COL_TYPE, storeDelegate);
  CheckBoxDelegate* storeLockDelegate = new CheckBoxDelegate();
  storeTreeView->setItemDelegateForColumn(STORE_COL_LOCK, storeLockDelegate);
  QObject::connect(storeTreeView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(onStoreTreeViewClicked(QModelIndex)));
  QObject::connect(storeTreeView,
                   SIGNAL(doubleClicked(QModelIndex)),
                   this,
                   SLOT(onStoreTreeViewDoubleClicked(QModelIndex)));

  QObject::connect(storeModel,
                   SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                   this,
                   SLOT(onStoreTreeViewDataChanged(QModelIndex, QModelIndex, QVector<int>)));

  auto* header = storeTreeView->header();
  header->setSectionsClickable(true);
  QObject::connect(header,
                   SIGNAL(sectionClicked(int)),
                   this,
                   SLOT(onStoreHeaderClicked(int)));

  storeTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

string MedUi::getLastDigit() {
  return mainWindow->findChild<QLineEdit*>("lastDigit")->text().toStdString();
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
  if (scanType == SCAN_TYPE_STRING) {
    scanValue = encodingManager->encode(scanValue);
  }

  scanUpdateMutex->lock();
  scanModel->clearAll();
  scanUpdateMutex->unlock();

  try {
    med->scan(scanValue, scanType, fastScan, getLastDigit());
  } catch(EmptyListException &ex) {
    statusBar->showMessage(ex.what());
    cerr << ex.what() << endl;
    return;
  } catch(MedException &ex) {
    cerr << "scan: "<< ex.what() << endl;
  }

  if(med->getScans().size() <= SCAN_ADDRESS_VISIBLE_SIZE) {
    scanUpdateMutex->lock();
    scanModel->addScan(scanType);
    scanUpdateMutex->unlock();
  }

  if (QString(scanValue.c_str()).trimmed() == "?") {
    statusBar->showMessage("Snapshot saved");
  }
  updateNumberOfAddresses();
  if (!med->getIsProcessPaused() && med->getCanResumeProcess()) {
    med->resumeProcess();
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
  if (scanType == SCAN_TYPE_STRING) {
    scanValue = encodingManager->encode(scanValue);
  }

  try {
    med->filter(scanValue, scanType, fastScan);
  } catch (EmptyListException &ex) {
    statusBar->showMessage(ex.what());
    cerr << ex.what() << endl;
    return;
  } catch (MedException &ex) {
    cerr << "filter: "<< ex.what() << endl;
  }

  if(med->getScans().size() <= SCAN_ADDRESS_VISIBLE_SIZE) {
    scanModel->addScan(scanType);
  }

  updateNumberOfAddresses();
  if (!med->getIsProcessPaused() && med->getCanResumeProcess()) {
    med->resumeProcess();
  }
}

void MedUi::onPauseCheckboxClicked(bool checked) {
  if (checked) {
    med->pauseProcess();
  }
  else {
    med->resumeProcess();
  }
}


void MedUi::onScanTreeViewClicked(const QModelIndex &index) {
  if(index.column() == SCAN_COL_TYPE) {
    scanTreeView->edit(index); //Trigger edit by 1 click
  }
}

void MedUi::onScanTreeViewDoubleClicked(const QModelIndex &index) {
  if (index.column() == SCAN_COL_VALUE) {
    scanUpdateMutex->lock();
    setScanState(UiState::Editing);
  }
}

void MedUi::onScanTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
  // qDebug() << topLeft << bottomRight << roles;
  if (topLeft.column() == SCAN_COL_VALUE) {
    tryUnlock(*scanUpdateMutex);
    setScanState(UiState::Idle);
  }
}

void MedUi::onStoreTreeViewDoubleClicked(const QModelIndex &index) {
  if (index.column() == STORE_COL_VALUE) {
    storeUpdateMutex.lock();
    storeState = UiState::Editing;
  }
}

void MedUi::onStoreTreeViewClicked(const QModelIndex &index) {
  if (index.column() == STORE_COL_TYPE) {
    storeTreeView->edit(index);
  }
  else if (index.column() == STORE_COL_LOCK) {
    storeTreeView->edit(index);
  }
}

void MedUi::onStoreTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
  if (topLeft.column() == STORE_COL_VALUE) {
    tryUnlock(storeUpdateMutex);
    storeState = UiState::Idle;
  }
}

void MedUi::onScanAddClicked() {
  auto indexes = scanTreeView
    ->selectionModel()
    ->selectedRows(SCAN_COL_ADDRESS);

  scanUpdateMutex->lock();
  for (int i = 0; i < indexes.size(); i++) {
    med->addToStoreByIndex(indexes[i].row());
  }

  storeModel->refresh();
  scanUpdateMutex->unlock();
}

void MedUi::onScanAddAllClicked() {
  scanUpdateMutex->lock();
  for (size_t i = 0; i < med->getScans().size(); i++) {
    med->addToStoreByIndex(i);
  }
  storeModel->refresh();
  scanUpdateMutex->unlock();
}

void MedUi::onScanClearClicked() {
  scanUpdateMutex->lock();
  scanModel->empty();
  scanUpdateMutex->unlock();
  statusBar->showMessage("Scan cleared");
}

void MedUi::onStoreHeaderClicked(int logicalIndex) {
  if (logicalIndex == STORE_COL_DESCRIPTION) {
    storeUpdateMutex.lock();
    storeModel->sortByDescription();
    storeUpdateMutex.unlock();
  }
  else if (logicalIndex == STORE_COL_ADDRESS) {
    storeUpdateMutex.lock();
    storeModel->sortByAddress();
    storeUpdateMutex.unlock();
  }
}

///////////////////
// Store buttons //
///////////////////

void MedUi::onStoreClearClicked() {
  storeUpdateMutex.lock();
  storeModel->empty();
  storeUpdateMutex.unlock();
}

void MedUi::onStoreNextClicked() {
  auto indexes = storeTreeView->selectionModel()->selectedRows(STORE_COL_ADDRESS);
  if (indexes.size() == 0) {
    cerr << "onStoreNextClicked: nothing selected" << endl;
    return;
  }

  storeUpdateMutex.lock();
  med->getStore()->addNextAddress(indexes[0].row());
  storeModel->addRow();
  storeUpdateMutex.unlock();
}

void MedUi::onStorePrevClicked() {
  auto indexes = storeTreeView->selectionModel()->selectedRows(STORE_COL_ADDRESS);
  if (indexes.size() == 0) {
    cerr << "onStorePrevClicked: nothing selected" << endl;
    return;
  }

  storeUpdateMutex.lock();
  med->getStore()->addPrevAddress(indexes[0].row());
  storeModel->addRow();
  storeUpdateMutex.unlock();
}


void storeShift(MedUi* mainUi, bool reverse = false) {
  auto mainWindow = mainUi->mainWindow;
  auto &med = mainUi->med;
  auto storeTreeView = mainUi->storeTreeView;
  auto &storeUpdateMutex = mainUi->storeUpdateMutex;
  auto storeModel = mainUi->storeModel;
  auto statusBar = mainUi->statusBar;

  long difference;
  try {
    long shiftFrom = hexToInt(mainWindow->findChild<QLineEdit*>("shiftFrom")->text().toStdString());
    long shiftTo = hexToInt(mainWindow->findChild<QLineEdit*>("shiftTo")->text().toStdString());
    if (reverse) {
      difference = shiftFrom - shiftTo;
    }
    else {
      difference = shiftTo - shiftFrom;
    }
  } catch(MedException &e) {
    cerr << e.what() << endl;
  }

  //Get PID
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }

  auto indexes = storeTreeView
    ->selectionModel()
    ->selectedRows(STORE_COL_ADDRESS);

  storeUpdateMutex.lock();
  for (auto i = 0; i < indexes.size(); i++) {
    med->getStore()->shiftAddress(indexes[i].row(), difference);
  }
  storeModel->refresh();
  storeUpdateMutex.unlock();
}

void MedUi::onStoreShiftClicked() {
  storeShift(this);
}

void MedUi::onStoreUnshiftClicked() {
  storeShift(this, true);
}

void MedUi::onStoreMoveClicked() {
  long moveSteps;
  try {
    moveSteps = mainWindow->findChild<QLineEdit*>("shiftTo")->text().toInt();
  } catch(MedException &e) {
    cerr << e.what() << endl;
  }

  //Get PID
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }

  auto indexes = storeTreeView
    ->selectionModel()
    ->selectedRows(STORE_COL_ADDRESS);

  storeUpdateMutex.lock();
  for (auto i = 0; i < indexes.size(); i++) {
    med->getStore()->shiftAddress(indexes[i].row(), moveSteps);
  }
  storeModel->refresh();
  storeUpdateMutex.unlock();
}


///////////////////
// Menu items    //
///////////////////

void MedUi::onSaveAsTriggered() {
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }
  QString filename = QFileDialog::getSaveFileName(mainWindow,
                                                  QString("Save JSON"),
                                                  "./",
                                                  QString("Save JSON (*.json)"));

  if (filename == "") {
    return;
  }
  this->filename = filename;
  setWindowTitle();

  med->saveFile(filename.toStdString().c_str());
}

void MedUi::onOpenTriggered() {
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }
  QString filename = QFileDialog::getOpenFileName(mainWindow,
                                                  QString("Open JSON"),
                                                  "./",
                                                  QString("Open JSON (*.json)"));
  if (filename == "") {
    return;
  }

  openFile(filename);
}

void MedUi::onReloadTriggered() {
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }
  if (filename == "") {
    return;
  }

  openFile(filename);
}

void MedUi::openFile(QString filename) {
  this->filename = filename;
  setWindowTitle();

  med->openFile(filename.toStdString().c_str());

  storeUpdateMutex.lock();
  storeModel->clearAll();
  storeModel->refresh();
  storeUpdateMutex.unlock();

  notesArea->setPlainText(QString::fromStdString(med->getNotes()));
}

void MedUi::setWindowTitle() {
  if (filename.length() > 0) {
    mainWindow->setWindowTitle(MAIN_TITLE + ": " + filename);
  }
  else {
    mainWindow->setWindowTitle(MAIN_TITLE);
  }
}

void MedUi::onSaveTriggered() {
  if (filename == "") {
    return;
  }
  med->saveFile(filename.toStdString().c_str());
  statusBar->showMessage("Saved");
}


////// End Menu > File ////////////

void MedUi::onShowNotesTriggered(bool checked) {
  if (checked) {
    notesArea->show();
  }
  else {
    notesArea->hide();
  }
}

void MedUi::onAutoRefreshTriggered(bool checked) {
  if (checked) {
    autoRefresh = true;
  } else {
    autoRefresh = false;
  }
}

void MedUi::onFastScanTriggered(bool checked) {
  if (checked) {
    fastScan = true;
  } else {
    fastScan = false;
  }
}

void MedUi::onResumeProcessTriggered(bool checked) {
  if (checked) {
    med->setCanResumeProcess(true);
  } else {
    med->setCanResumeProcess(false);
  }
}

void MedUi::onNotesAreaChanged() {
  med->setNotes(notesArea->toPlainText().toStdString());
}

void MedUi::onQuitTriggered() {
  app->quit();
}


//////////////////
// Edit address //
//////////////////


void MedUi::onNewAddressTriggered() {
  if(med->selectedProcess.pid == "") {
    statusBar->showMessage("No process selected");
    return;
  }
  storeUpdateMutex.lock();
  med->addNewAddress();
  storeModel->addRow();
  storeUpdateMutex.unlock();
}

void MedUi::onDeleteAddressTriggered() {
  auto indexes = storeTreeView
    ->selectionModel()
    ->selectedRows(STORE_COL_ADDRESS);

  //Sort and reverse
  sort(indexes.begin(), indexes.end(), [](QModelIndex a, QModelIndex b) {
      return a.row() > b.row();
    });

  storeUpdateMutex.lock();
  for (auto i = 0; i < indexes.size(); i++) {
    med->getStore()->deleteAddress(indexes[i].row());
  }
  storeModel->refresh();
  storeUpdateMutex.unlock();
}

void MedUi::onMemEditorTriggered() {
  memEditor->show();
}

void MedUi::updateNumberOfAddresses() {
  char message[128];
  sprintf(message, "%ld", med->getScans().size());
  mainWindow->findChild<QLabel*>("found")->setText(message);
}

void MedUi::refresh(MedUi* mainUi) {
  // TODO: Store refresh if closing
  while (1) {
    if (mainUi->autoRefresh) {
      mainUi->refreshScanTreeView();
      mainUi->refreshStoreTreeView();
    }
    std::this_thread::sleep_for(chrono::milliseconds(REFRESH_RATE));
  }
}

void MedUi::onRefreshTriggered() {
  refreshScanTreeView();
  refreshStoreTreeView();
}

void MedUi::refreshScanTreeView() {
  scanUpdateMutex->lock();
  try {
    scanModel->refreshValues();
  } catch (MedException& ex) {
    scanUpdateMutex->unlock();
    cerr << ex.getMessage() << endl;
  }
  scanUpdateMutex->unlock();
}

void MedUi::refreshStoreTreeView() {
  storeUpdateMutex.lock();
  try {
    storeModel->refreshValues();
  } catch (MedException& ex) {
    storeUpdateMutex.unlock();
    cerr << ex.getMessage() << endl;
  }
  storeUpdateMutex.unlock();
}

UiState MedUi::getScanState() {
  return scanState;
}

UiState MedUi::getStoreState() {
  return storeState;
}

void MedUi::setScanState(UiState state) {
  scanState = state;
}

void MedUi::setStoreState(UiState state) {
  storeState = state;
}

void MedUi::onScopeStartEdited() {
  string start = mainWindow->findChild<QLineEdit*>("scopeStart")->text().toStdString();
  if (start.size() == 0) {
    med->setScopeStart(0);
  }
  else {
    med->setScopeStart(hexToInt(start));
  }
}

void MedUi::onScopeEndEdited() {
  string end = mainWindow->findChild<QLineEdit*>("scopeEnd")->text().toStdString();
  if (end.size() == 0) {
    med->setScopeEnd(0);
  }
  else {
    med->setScopeEnd(hexToInt(end));
  }
}
