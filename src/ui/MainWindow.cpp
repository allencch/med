#include <iostream>
#include <algorithm>
#include <QDebug>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QStatusBar>
#include <QtUiTools/QUiLoader>
#include <QFile>
#include <QAction>
#include <QCheckBox>
#include <QFileDialog>
#include <QActionGroup>

#include "ui/MainWindow.hpp"
#include "ui/ComboBoxDelegate.hpp"
#include "ui/CheckBoxDelegate.hpp"
#include "ui/MemEditor.hpp"
#include "med/MedCommon.hpp"
#include "med/MemOperator.hpp"

const QString MAIN_TITLE = "Med UI";

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();

    worker_ = new MedWorker();
    worker_->moveToThread(&workerThread_);

    connectSignals();

    workerThread_.start();
}

MainWindow::~MainWindow() {
    workerThread_.quit();
    workerThread_.wait();
    delete worker_;
}

void MainWindow::setupUi() {
    QUiLoader loader;
    QFile file(":/ui/main-qt.ui");
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open UI file");
        return;
    }
    QWidget* uiWidget = loader.load(&file);
    file.close();

    if (!uiWidget) {
        QMessageBox::critical(this, "Error", "Could not load UI");
        return;
    }

    QMainWindow* uiWindow = qobject_cast<QMainWindow*>(uiWidget);
    if (uiWindow) {
        // Migration
        setCentralWidget(uiWindow->takeCentralWidget());

        // Take the menu bar
        QMenuBar* uiMenuBar = uiWindow->menuBar();
        setMenuBar(uiMenuBar);

        // IMPORTANT: Move all actions to this window so shortcuts/hotkeys work
        QList<QAction*> allActions = uiWindow->findChildren<QAction*>();
        for (QAction* action : allActions) {
            action->setParent(this);
            this->addAction(action);
            action->setShortcutContext(Qt::WindowShortcut);
        }

        setStatusBar(uiWindow->statusBar());
        this->resize(uiWindow->size());
    } else {
        setCentralWidget(uiWidget);
        this->resize(uiWidget->size());
    }

    setWindowTitle(MAIN_TITLE);

    // Finding components
    scanTreeView_ = findChild<QTreeView*>("scanTreeView");
    if (scanTreeView_) scanTreeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    storeTreeView_ = findChild<QTreeView*>("storeTreeView");
    if (storeTreeView_) storeTreeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    scanValueEdit_ = findChild<QLineEdit*>("scanEntry");
    lastDigitEdit_ = findChild<QLineEdit*>("lastDigit");
    scanTypeCombo_ = findChild<QComboBox*>("scanType");
    foundLabel_ = findChild<QLabel*>("found");
    notesEdit_ = findChild<QPlainTextEdit*>("notes");
    selectedProcessEdit_ = findChild<QLineEdit*>("selectedProcess");
    shiftFromEdit_ = findChild<QLineEdit*>("shiftFrom");
    shiftToEdit_ = findChild<QLineEdit*>("shiftTo");

    scanButton_ = findChild<QPushButton*>("scanButton");
    filterButton_ = findChild<QPushButton*>("filterButton");
    QPushButton* processButton = findChild<QPushButton*>("process");
    QPushButton* scanAddButton = findChild<QPushButton*>("scanAdd");
    QPushButton* scanAddAllButton = findChild<QPushButton*>("scanAddAll");
    QPushButton* scanClearButton = findChild<QPushButton*>("scanClear");
    QCheckBox* pauseCheckbox = findChild<QCheckBox*>("pauseCheckbox");

    QPushButton* storeShiftButton = findChild<QPushButton*>("storeShift");
    QPushButton* storeUnshiftButton = findChild<QPushButton*>("storeUnshift");
    QPushButton* moveAddressButton = findChild<QPushButton*>("moveAddress");

    if (scanButton_) connect(scanButton_, &QPushButton::clicked, this, &MainWindow::onScanClicked);
    if (filterButton_) connect(filterButton_, &QPushButton::clicked, this, &MainWindow::onFilterClicked);
    if (processButton) connect(processButton, &QPushButton::clicked, this, &MainWindow::onSelectProcessClicked);
    if (scanAddButton) connect(scanAddButton, &QPushButton::clicked, this, &MainWindow::onAddToStoreClicked);
    if (scanAddAllButton) connect(scanAddAllButton, &QPushButton::clicked, this, &MainWindow::onAddAllToStoreClicked);
    if (scanClearButton) connect(scanClearButton, &QPushButton::clicked, this, &MainWindow::onScanClearClicked);
    if (pauseCheckbox) connect(pauseCheckbox, &QCheckBox::clicked, this, &MainWindow::onPauseClicked);

    if (storeShiftButton) connect(storeShiftButton, &QPushButton::clicked, this, &MainWindow::onStoreShiftClicked);
    if (storeUnshiftButton) connect(storeUnshiftButton, &QPushButton::clicked, this, &MainWindow::onStoreUnshiftClicked);
    if (moveAddressButton) connect(moveAddressButton, &QPushButton::clicked, this, &MainWindow::onMoveAddressClicked);

    QPushButton* prevButton = findChild<QPushButton*>("prevAddress");
    QPushButton* nextButton = findChild<QPushButton*>("nextAddress");
    if (prevButton) connect(prevButton, &QPushButton::clicked, this, &MainWindow::onPrevAddressClicked);
    if (nextButton) connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextAddressClicked);

    // Setup Models
    scanModel_ = new QStandardItemModel(this);
    scanModel_->setHorizontalHeaderLabels({"Address", "Type", "Value"});
    if (scanTreeView_) {
        scanTreeView_->setModel(scanModel_);
        scanTreeView_->setItemDelegateForColumn(1, new ComboBoxDelegate(this));
        connect(scanTreeView_, &QTreeView::doubleClicked, this, &MainWindow::onScanTreeViewDoubleClicked);
        connect(scanModel_, &QStandardItemModel::dataChanged, this, &MainWindow::onScanDataChanged);
        connect(scanTreeView_, &QTreeView::clicked, this, [this](const QModelIndex& index) {
            if (index.column() == 1) { // Type column
                scanTreeView_->edit(index);
            }
        });
    }

    storeModel_ = new QStandardItemModel(this);
    storeModel_->setHorizontalHeaderLabels({"Description +", "Address +", "Type", "Value", "Lock"});
    if (storeTreeView_) {
        storeTreeView_->setModel(storeModel_);
        storeTreeView_->setItemDelegateForColumn(2, new ComboBoxDelegate(this));
        storeTreeView_->setItemDelegateForColumn(4, new CheckBoxDelegate(this));
        connect(storeModel_, &QStandardItemModel::dataChanged, this, &MainWindow::onStoreDataChanged);
        connect(storeTreeView_, &QTreeView::clicked, this, [this](const QModelIndex& index) {
            if (index.column() == 2) { // Type column
                storeTreeView_->edit(index);
            }
        });
        storeTreeView_->header()->setSectionsClickable(true);
        connect(storeTreeView_->header(), &QHeaderView::sectionClicked, this, &MainWindow::onStoreHeaderClicked);
    }

    processDialog_ = new ProcessDialog(this);
    connect(processDialog_, &ProcessDialog::processSelected, this, &MainWindow::onProcessSelected);

    QLineEdit* scopeStartEdit = findChild<QLineEdit*>("scopeStart");
    QLineEdit* scopeEndEdit = findChild<QLineEdit*>("scopeEnd");
    if (scopeStartEdit) connect(scopeStartEdit, &QLineEdit::editingFinished, this, [this, scopeStartEdit]() {
        try {
            Address start = 0;
            QString text = scopeStartEdit->text().trimmed();
            if (!text.isEmpty()) {
                start = MedUtil::hexToInt(text.toStdString());
            }
            QMetaObject::invokeMethod(worker_, "setScopeStart", Qt::QueuedConnection, Q_ARG(Address, start));
        } catch (...) {}
    });
    if (scopeEndEdit) connect(scopeEndEdit, &QLineEdit::editingFinished, this, [this, scopeEndEdit]() {
        try {
            Address end = 0;
            QString text = scopeEndEdit->text().trimmed();
            if (!text.isEmpty()) {
                end = MedUtil::hexToInt(text.toStdString());
            }
            QMetaObject::invokeMethod(worker_, "setScopeEnd", Qt::QueuedConnection, Q_ARG(Address, end));
        } catch (...) {}
    });

    // Menu Actions
    QAction* actionOpen = findChild<QAction*>("actionOpen");
    QAction* actionSave = findChild<QAction*>("actionSave");
    QAction* actionSaveAs = findChild<QAction*>("actionSaveAs");
    QAction* actionReload = findChild<QAction*>("actionReload");
    QAction* actionNew = findChild<QAction*>("actionNewAddress");
    QAction* actionDelete = findChild<QAction*>("actionDeleteAddress");
    QAction* actionMemEditor = findChild<QAction*>("actionMemEditor");
    QAction* actionUnlockAll = findChild<QAction*>("actionUnlockAll");
    QAction* actionShowNotes = findChild<QAction*>("actionShowNotes");
    QAction* actionFastScan = findChild<QAction*>("actionFastScan");
    QAction* actionCanResume = findChild<QAction*>("actionResumeProcess");
    QAction* actionAutoRefresh = findChild<QAction*>("actionAutoRefresh");
    QAction* actionForceResume = findChild<QAction*>("actionForceResume");
    QAction* actionQuit = findChild<QAction*>("actionQuit");
    QAction* actionRefresh = findChild<QAction*>("actionRefresh");
    QAction* actionDefaultEncoding = findChild<QAction*>("actionDefaultEncoding");
    QAction* actionBig5Encoding = findChild<QAction*>("actionBig5Encoding");
    QAction* actionStoreClear = findChild<QAction*>("actionStoreClear");

    if (actionOpen) connect(actionOpen, &QAction::triggered, this, &MainWindow::onOpenTriggered);
    if (actionSave) connect(actionSave, &QAction::triggered, this, &MainWindow::onSaveTriggered);
    if (actionSaveAs) connect(actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAsTriggered);
    if (actionReload) connect(actionReload, &QAction::triggered, this, &MainWindow::onReloadTriggered);
    if (actionMemEditor) connect(actionMemEditor, &QAction::triggered, this, &MainWindow::onMemEditorTriggered);
    if (actionQuit) connect(actionQuit, &QAction::triggered, this, &MainWindow::close);
    if (actionRefresh) connect(actionRefresh, &QAction::triggered, this, &MainWindow::onRefreshRequested);
    if (actionNew) connect(actionNew, &QAction::triggered, this, &MainWindow::onNewAddressTriggered);
    if (actionDelete) connect(actionDelete, &QAction::triggered, this, &MainWindow::onDeleteAddressTriggered);
    if (actionUnlockAll) connect(actionUnlockAll, &QAction::triggered, this, &MainWindow::onUnlockAllTriggered);
    if (actionShowNotes) connect(actionShowNotes, &QAction::triggered, this, &MainWindow::onShowNotesTriggered);
    if (actionFastScan) connect(actionFastScan, &QAction::triggered, this, &MainWindow::onFastScanTriggered);
    if (actionCanResume) connect(actionCanResume, &QAction::triggered, this, &MainWindow::onCanResumeTriggered);
    if (actionAutoRefresh) connect(actionAutoRefresh, &QAction::triggered, this, &MainWindow::onAutoRefreshTriggered);
    if (actionForceResume) connect(actionForceResume, &QAction::triggered, this, &MainWindow::onForceResumeTriggered);
    if (actionDefaultEncoding) connect(actionDefaultEncoding, &QAction::triggered, this, &MainWindow::onDefaultEncodingTriggered);
    if (actionBig5Encoding) connect(actionBig5Encoding, &QAction::triggered, this, &MainWindow::onBig5EncodingTriggered);
    if (actionStoreClear) connect(actionStoreClear, &QAction::triggered, this, &MainWindow::onStoreClearTriggered);

    // Grouping the actions to ensure toggle behavior
    if (actionDefaultEncoding && actionBig5Encoding) {
        QActionGroup* encodingGroup = new QActionGroup(this);
        encodingGroup->addAction(actionDefaultEncoding);
        encodingGroup->addAction(actionBig5Encoding);
        encodingGroup->setExclusive(true);
        actionDefaultEncoding->setChecked(true);
    }

    // Named Scans
    namedScanCombo_ = findChild<QComboBox*>("namedScans");
    namedScanNameEdit_ = findChild<QLineEdit*>("namedScan_name");
    namedScanAddBtn_ = findChild<QPushButton*>("namedScan_add");
    namedScanDeleteBtn_ = findChild<QPushButton*>("namedScan_delete");

    if (namedScanAddBtn_) connect(namedScanAddBtn_, &QPushButton::clicked, this, &MainWindow::onNamedScanAddClicked);
    if (namedScanDeleteBtn_) connect(namedScanDeleteBtn_, &QPushButton::clicked, this, &MainWindow::onNamedScanDeleteClicked);
    if (namedScanCombo_) {
        connect(namedScanCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::onNamedScanComboBoxChanged);
        namedScanCombo_->clear();
        namedScanCombo_->addItem(med::NamedScans::DEFAULT_NAME);
    }

    memEditor_ = new MemEditor(this);

    // Initial State
    if (notesEdit_) notesEdit_->hide();
    if (scanTypeCombo_) scanTypeCombo_->setCurrentText("int32");
    }

void MainWindow::connectSignals() {
    connect(worker_, &MedWorker::scanCompleted, this, &MainWindow::onScanCompleted);
    connect(worker_, &MedWorker::filterCompleted, this, &MainWindow::onFilterCompleted);
    connect(worker_, &MedWorker::watchedValuesRefreshed, this, &MainWindow::onWatchedValuesRefreshed);
    connect(worker_, &MedWorker::refreshRequested, this, &MainWindow::onRefreshRequested);
    connect(worker_, &MedWorker::processListReady, this, &MainWindow::onProcessListReady);
    connect(worker_, &MedWorker::fileLoaded, this, &MainWindow::onFileLoaded);
    connect(worker_, &MedWorker::memoryReady, memEditor_, &MemEditor::onMemoryReady);
    connect(worker_, &MedWorker::errorOccurred, this, &MainWindow::onError);
}

void MainWindow::onSelectProcessClicked() {
    worker_->requestProcessList();
}

void MainWindow::onProcessSelected(pid_t pid, const QString& name) {
    currentPid_ = pid;
    if (selectedProcessEdit_) selectedProcessEdit_->setText(QString("%1 %2").arg(pid).arg(name));
    QMetaObject::invokeMethod(worker_, "setPid", Qt::QueuedConnection, Q_ARG(pid_t, pid));
    statusBar()->showMessage(QString("Attached to %1 (%2)").arg(name).arg(pid));
}

void MainWindow::onScanClicked() {
    if (currentPid_ == 0) {
        QMessageBox::warning(this, "Process", "Select a process first");
        return;
    }

    QString val = scanValueEdit_->text();
    if (val.isEmpty()) return;

    ScanType type = MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString());
    ScanParser::OpType op = ScanParser::getOpType(val.toStdString());

    std::vector<int> lastDigits;
    if (lastDigitEdit_ && !lastDigitEdit_->text().isEmpty()) {
        auto tokens = StringUtil::split(lastDigitEdit_->text().toStdString(), ',');
        for (const auto& t : tokens) {
            try {
                lastDigits.push_back((int)MedUtil::hexToInt(t));
            } catch (...) {}
        }
    }

    namedScans_.setActiveType(type);

    QMetaObject::invokeMethod(worker_, "startScan", Qt::QueuedConnection,
                              Q_ARG(QString, val),
                              Q_ARG(ScanType, type),
                              Q_ARG(ScanParser::OpType, op),
                              Q_ARG(bool, fastScan_),
                              Q_ARG(std::vector<int>, lastDigits));

    if (scanButton_) scanButton_->setEnabled(false);
    if (filterButton_) filterButton_->setEnabled(false);
    statusBar()->showMessage("Scanning...");
}

void MainWindow::onFilterClicked() {
    QString val = scanValueEdit_->text();
    if (val.isEmpty()) return;

    ScanType type = MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString());
    ScanParser::OpType op = ScanParser::getOpType(val.toStdString());

    std::vector<int> lastDigits;
    if (lastDigitEdit_ && !lastDigitEdit_->text().isEmpty()) {
        auto tokens = StringUtil::split(lastDigitEdit_->text().toStdString(), ',');
        for (const auto& t : tokens) {
            try {
                lastDigits.push_back((int)MedUtil::hexToInt(t));
            } catch (...) {}
        }
    }

    namedScans_.setActiveType(type);

    QMetaObject::invokeMethod(worker_, "startFilter", Qt::QueuedConnection,
                              Q_ARG(std::vector<ScanResult>, namedScans_.getActiveResults()),
                              Q_ARG(QString, val),
                              Q_ARG(ScanType, type),
                              Q_ARG(ScanParser::OpType, op),
                              Q_ARG(bool, fastScan_),
                              Q_ARG(std::vector<int>, lastDigits));

    if (scanButton_) scanButton_->setEnabled(false);
    if (filterButton_) filterButton_->setEnabled(false);
    statusBar()->showMessage("Filtering...");
}

void MainWindow::onScanCompleted(const std::vector<ScanResult>& results) {
    isProgrammaticUpdate_ = true;
    bool sizeChanged = (results.size() != namedScans_.getActiveResults().size());
    namedScans_.setActiveResults(results);

    if (sizeChanged) {
        updateScanModel(results);
    } else {
        // Just update values
        size_t count = std::min(results.size(), (size_t)800);
        for (size_t i = 0; i < count; ++i) {
            const auto& res = results[i];

            // Address
            QString newAddr = QString::fromStdString(MedUtil::intToHex(res.address));
            if (scanModel_->data(scanModel_->index(i, 0)).toString() != newAddr) {
                scanModel_->setData(scanModel_->index(i, 0), newAddr, Qt::DisplayRole);
            }

            // Type
            QString newType = QString::fromStdString(MedUtil::scanTypeToString(res.type));
            if (scanModel_->data(scanModel_->index(i, 1)).toString() != newType) {
                scanModel_->setData(scanModel_->index(i, 1), newType, Qt::DisplayRole);
            }

            // Value
            const SizedBytes& displayData = res.liveData.isEmpty() ? res.data : res.liveData;
            QString newVal = QString::fromStdString(MemOperator::toString(displayData.getBytes(), displayData.getSize(), res.type, encoding_));
            auto index = scanModel_->index(i, 2);
            if (scanModel_->data(index, Qt::EditRole).toString() != newVal) {
                scanModel_->setData(index, newVal, Qt::DisplayRole);
            }
        }
    }

    if (foundLabel_) foundLabel_->setText(QString::number(results.size()));
    if (results.empty() && scanValueEdit_->text().contains("?")) {
        statusBar()->showMessage("Snapshot saved");
    } else {
        statusBar()->showMessage(QString("Found %1 addresses").arg(results.size()));
    }

    if (scanButton_) scanButton_->setEnabled(true);
    if (filterButton_) filterButton_->setEnabled(true);
    isProgrammaticUpdate_ = false;
}

void MainWindow::onFilterCompleted(const std::vector<ScanResult>& results) {
    onScanCompleted(results);
}

void MainWindow::onWatchedValuesRefreshed(const std::vector<WatchedAddress>& watched) {
    isProgrammaticUpdate_ = true;
    bool structureChanged = (watched.size() != watchedAddresses_.size());
    if (!structureChanged) {
        for (int i = 0; i < (int)watched.size(); ++i) {
            if (watched[i].address != watchedAddresses_[i].address ||
                watched[i].type != watchedAddresses_[i].type ||
                watched[i].description != watchedAddresses_[i].description) {
                structureChanged = true;
                break;
            }
        }
    }

    watchedAddresses_ = watched;

    if (structureChanged) {
        updateStoreModel();
    } else {
        for (int i = 0; i < (int)watched.size(); ++i) {
            auto index = storeModel_->index(i, 3);
            if (storeModel_->data(index, Qt::EditRole).toString() != QString::fromStdString(watched[i].value)) {
                storeModel_->setData(index, QString::fromStdString(watched[i].value), Qt::DisplayRole);
            }
        }
    }
    isProgrammaticUpdate_ = false;
}

void MainWindow::onRefreshRequested() {
    if (!namedScans_.getActiveResults().empty() && namedScans_.getActiveResults().size() <= 800) {
        QMetaObject::invokeMethod(worker_, "refreshScanResults", Qt::QueuedConnection,
                                  Q_ARG(std::vector<ScanResult>, namedScans_.getActiveResults()));
    }
    QMetaObject::invokeMethod(worker_, "refreshWatchedValues", Qt::QueuedConnection);
}

void MainWindow::onProcessListReady(const std::vector<Process>& processes) {
    processDialog_->setProcessList(processes);
    processDialog_->show();
}

void MainWindow::onScanClearClicked() {
    namedScans_.getActiveResults().clear();
    scanModel_->clear();
    scanModel_->setHorizontalHeaderLabels({"Address", "Type", "Value"});
    if (foundLabel_) foundLabel_->setText("0");
    statusBar()->showMessage("Scan cleared");
}

void MainWindow::addWatchedAddress(const WatchedAddress& wa) {
    watchedAddresses_.push_back(wa);

    QList<QStandardItem*> items;
    items << new QStandardItem(QString::fromStdString(wa.description));
    items << new QStandardItem(QString::fromStdString(MedUtil::intToHex(wa.address)));
    items << new QStandardItem(QString::fromStdString(MedUtil::scanTypeToString(wa.type)));
    items << new QStandardItem(QString::fromStdString(wa.value));
    QStandardItem* lockItem = new QStandardItem();
    lockItem->setData(wa.locked, Qt::EditRole);
    items << lockItem;
    storeModel_->appendRow(items);
}

void MainWindow::onAddToStoreClicked() {
    auto selection = scanTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    // Sort to maintain order if needed, but not strictly required for addition
    std::vector<int> rows;
    for (const auto& index : selection) rows.push_back(index.row());
    std::sort(rows.begin(), rows.end());

    for (int row : rows) {
        if (row >= (int)namedScans_.getActiveResults().size()) continue;

        const auto& res = namedScans_.getActiveResults()[row];

        WatchedAddress wa;
        wa.description = "New Address";
        wa.address = res.address;
        wa.type = res.type;
        wa.value = MemOperator::toString(res.data.getBytes(), res.data.getSize(), res.type, encoding_);
        wa.locked = false;
        wa.lockValue = wa.value;
        addWatchedAddress(wa);
    }

    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onAddAllToStoreClicked() {
    for (const auto& res : namedScans_.getActiveResults()) {
        WatchedAddress wa;
        wa.description = "New Address";
        wa.address = res.address;
        wa.type = res.type;
        wa.value = MemOperator::toString(res.data.getBytes(), res.data.getSize(), res.type, encoding_);
        wa.locked = false;
        wa.lockValue = wa.value;
        addWatchedAddress(wa);
    }

    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}


void MainWindow::onScanTreeViewDoubleClicked(const QModelIndex& index) {
}

void MainWindow::onScanDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    if (isProgrammaticUpdate_) return;
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        if (row >= (int)namedScans_.getActiveResults().size()) continue;

        auto& res = namedScans_.getActiveResults()[row];

        if (topLeft.column() == 0) { // Address changed
            try {
                res.address = MedUtil::hexToInt(scanModel_->data(scanModel_->index(row, 0)).toString().toStdString());
            } catch (...) {}
        } else if (topLeft.column() == 1) { // Type changed
            res.type = MedUtil::stringToScanType(scanModel_->data(scanModel_->index(row, 1)).toString().toStdString());
        } else if (topLeft.column() == 2) { // Value changed
            QString newVal = scanModel_->data(scanModel_->index(row, 2)).toString();
            QMetaObject::invokeMethod(worker_, "writeMemory", Qt::QueuedConnection,
                                      Q_ARG(Address, res.address),
                                      Q_ARG(QString, newVal),
                                      Q_ARG(ScanType, res.type));
        }
    }
}

void MainWindow::onStoreTreeViewDoubleClicked(const QModelIndex& index) {
}

void MainWindow::onStoreDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    if (isProgrammaticUpdate_) return;
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        if (row >= (int)watchedAddresses_.size()) continue;

        auto& wa = watchedAddresses_[row];
        wa.description = storeModel_->data(storeModel_->index(row, 0)).toString().toStdString();
        try {
            wa.address = MedUtil::hexToInt(storeModel_->data(storeModel_->index(row, 1)).toString().toStdString());
        } catch (...) {}
        wa.type = MedUtil::stringToScanType(storeModel_->data(storeModel_->index(row, 2)).toString().toStdString());

        if (topLeft.column() == 3) { // Value changed
            QString newVal = storeModel_->data(storeModel_->index(row, 3)).toString();
            if (newVal == "??") continue;
            wa.value = newVal.toStdString();
            if (wa.locked) wa.lockValue = wa.value;
            QMetaObject::invokeMethod(worker_, "writeMemory", Qt::QueuedConnection,
                                      Q_ARG(Address, wa.address),
                                      Q_ARG(QString, newVal),
                                      Q_ARG(ScanType, wa.type));
        } else if (topLeft.column() == 4) { // Lock toggled
            wa.locked = storeModel_->data(storeModel_->index(row, 4), Qt::EditRole).toBool();
            if (wa.locked) {
                wa.lockValue = storeModel_->data(storeModel_->index(row, 3)).toString().toStdString();
            }
        }
    }
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onStoreHeaderClicked(int logicalIndex) {
    qDebug() << "Header clicked: " << logicalIndex;
    if (logicalIndex == 0) { // Description
        std::sort(watchedAddresses_.begin(), watchedAddresses_.end(), [](const WatchedAddress& a, const WatchedAddress& b) {
            return a.description < b.description;
        });
    } else if (logicalIndex == 1) { // Address
        std::sort(watchedAddresses_.begin(), watchedAddresses_.end(), [](const WatchedAddress& a, const WatchedAddress& b) {
            return a.address < b.address;
        });
    } else {
        return;
    }
    updateStoreModel();
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::updateStoreModel() {
    isProgrammaticUpdate_ = true;
    storeModel_->removeRows(0, storeModel_->rowCount());
    for (const auto& wa : watchedAddresses_) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::fromStdString(wa.description));
        items << new QStandardItem(QString::fromStdString(MedUtil::intToHex(wa.address)));
        items << new QStandardItem(QString::fromStdString(MedUtil::scanTypeToString(wa.type)));
        items << new QStandardItem(QString::fromStdString(wa.value));
        QStandardItem* lockItem = new QStandardItem();
        lockItem->setData(wa.locked, Qt::EditRole);
        items << lockItem;
        storeModel_->appendRow(items);
    }
    isProgrammaticUpdate_ = false;
}

void MainWindow::onPauseClicked(bool checked) {
    QMetaObject::invokeMethod(worker_, "setProcessPaused", Qt::QueuedConnection, Q_ARG(bool, checked));
}

void MainWindow::onCanResumeTriggered(bool checked) {
    QMetaObject::invokeMethod(worker_, "setCanResume", Qt::QueuedConnection, Q_ARG(bool, checked));
}

void MainWindow::onAutoRefreshTriggered(bool checked) {
    autoRefresh_ = checked;
    QMetaObject::invokeMethod(worker_, "setAutoRefresh", Qt::QueuedConnection, Q_ARG(bool, checked));
}

void MainWindow::onForceResumeTriggered(bool checked) {
    forceResume_ = checked;
    QMetaObject::invokeMethod(worker_, "setForceResume", Qt::QueuedConnection, Q_ARG(bool, checked));
}

void MainWindow::onFastScanTriggered(bool checked) {
    fastScan_ = checked;
}

void MainWindow::onOpenTriggered() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open JSON", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        currentFilename_ = fileName;
        QMetaObject::invokeMethod(worker_, "loadFile", Qt::QueuedConnection, Q_ARG(QString, fileName));
    }
}

void MainWindow::onSaveTriggered() {
    if (currentFilename_.isEmpty()) {
        onSaveAsTriggered();
    } else {
        QString notes = notesEdit_ ? notesEdit_->toPlainText() : "";
        QMetaObject::invokeMethod(worker_, "saveFile", Qt::QueuedConnection, Q_ARG(QString, currentFilename_), Q_ARG(QString, notes));
        setWindowTitle(MAIN_TITLE + ": " + currentFilename_);
    }
}

void MainWindow::onSaveAsTriggered() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save JSON", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        currentFilename_ = fileName;
        onSaveTriggered();
    }
}

void MainWindow::onReloadTriggered() {
    if (!currentFilename_.isEmpty()) {
        QMetaObject::invokeMethod(worker_, "loadFile", Qt::QueuedConnection, Q_ARG(QString, currentFilename_));
    }
}

void MainWindow::onShowNotesTriggered(bool checked) {
    if (notesEdit_) notesEdit_->setVisible(checked);
}

void MainWindow::onMemEditorTriggered() {
    if (memEditor_) {
        memEditor_->show();
        memEditor_->raise();
        memEditor_->activateWindow();

        // Center it
        auto geo = geometry();
        memEditor_->move(geo.center() - memEditor_->rect().center());
    }
}

void MainWindow::onNewAddressTriggered() {
    WatchedAddress wa;
    wa.description = "New Address";
    wa.address = 0;
    wa.type = ScanType::UInt32;
    wa.value = "??";
    wa.locked = false;
    wa.lockValue = wa.value;

    addWatchedAddress(wa);

    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onDeleteAddressTriggered() {
    auto selection = storeTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    // Sort reverse to avoid index shift issues
    std::vector<int> rows;
    for (const auto& index : selection) rows.push_back(index.row());
    std::sort(rows.rbegin(), rows.rend());

    for (int row : rows) {
        storeModel_->removeRow(row);
        watchedAddresses_.erase(watchedAddresses_.begin() + row);
    }

    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onStoreClearTriggered() {
    watchedAddresses_.clear();
    updateStoreModel();
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
    statusBar()->showMessage("Store cleared");
}

void MainWindow::onUnlockAllTriggered() {
    for (int i = 0; i < (int)watchedAddresses_.size(); ++i) {
        watchedAddresses_[i].locked = false;
        storeModel_->setData(storeModel_->index(i, 4), false, Qt::EditRole);
    }
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onPrevAddressClicked() {
    auto selection = storeTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    std::vector<int> rows;
    for (const auto& index : selection) rows.push_back(index.row());
    std::sort(rows.begin(), rows.end());

    for (int row : rows) {
        if (row >= (int)watchedAddresses_.size()) continue;
        const auto& sourceWa = watchedAddresses_[row];
        WatchedAddress wa = sourceWa;
        size_t size = MedUtil::scanTypeToSize(wa.type);
        wa.address -= size;
        wa.locked = false;
        addWatchedAddress(wa);
    }
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onNextAddressClicked() {
    auto selection = storeTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    std::vector<int> rows;
    for (const auto& index : selection) rows.push_back(index.row());
    std::sort(rows.begin(), rows.end());

    for (int row : rows) {
        if (row >= (int)watchedAddresses_.size()) continue;
        const auto& sourceWa = watchedAddresses_[row];
        WatchedAddress wa = sourceWa;
        size_t size = MedUtil::scanTypeToSize(wa.type);
        wa.address += size;
        wa.locked = false;
        addWatchedAddress(wa);
    }
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onStoreShiftClicked() {
    auto selection = storeTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    QString fromText = shiftFromEdit_->text();
    QString toText = shiftToEdit_->text();
    if (fromText.isEmpty() || toText.isEmpty()) return;

    try {
        Address from = MedUtil::hexToInt(fromText.toStdString());
        Address to = MedUtil::hexToInt(toText.toStdString());

        for (const auto& index : selection) {
            int row = index.row();
            if (row >= (int)watchedAddresses_.size()) continue;
            watchedAddresses_[row].address = watchedAddresses_[row].address - from + to;
            watchedAddresses_[row].locked = false;
            watchedAddresses_[row].value = "??";
            storeModel_->setData(storeModel_->index(row, 1), QString::fromStdString(MedUtil::intToHex(watchedAddresses_[row].address)), Qt::EditRole);
            storeModel_->setData(storeModel_->index(row, 3), "??", Qt::EditRole);
            storeModel_->setData(storeModel_->index(row, 4), false, Qt::EditRole);
        }

        QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                                  Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
    } catch (...) {
        QMessageBox::warning(this, "Shift", "Invalid hexadecimal address");
    }
}

void MainWindow::onStoreUnshiftClicked() {
    auto selection = storeTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    QString fromText = shiftFromEdit_->text();
    QString toText = shiftToEdit_->text();
    if (fromText.isEmpty() || toText.isEmpty()) return;

    try {
        Address from = MedUtil::hexToInt(fromText.toStdString());
        Address to = MedUtil::hexToInt(toText.toStdString());

        for (const auto& index : selection) {
            int row = index.row();
            if (row >= (int)watchedAddresses_.size()) continue;
            watchedAddresses_[row].address = watchedAddresses_[row].address - to + from;
            watchedAddresses_[row].locked = false;
            watchedAddresses_[row].value = "??";
            storeModel_->setData(storeModel_->index(row, 1), QString::fromStdString(MedUtil::intToHex(watchedAddresses_[row].address)), Qt::EditRole);
            storeModel_->setData(storeModel_->index(row, 3), "??", Qt::EditRole);
            storeModel_->setData(storeModel_->index(row, 4), false, Qt::EditRole);
        }

        QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                                  Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
    } catch (...) {
        QMessageBox::warning(this, "Unshift", "Invalid hexadecimal address");
    }
}

void MainWindow::onMoveAddressClicked() {
    auto selection = storeTreeView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    QString bytesText = shiftToEdit_->text();
    if (bytesText.isEmpty()) return;

    try {
        long long offset = std::stoll(bytesText.toStdString(), nullptr, 0);

        for (const auto& index : selection) {
            int row = index.row();
            if (row >= (int)watchedAddresses_.size()) continue;
            watchedAddresses_[row].address += offset;
            watchedAddresses_[row].locked = false;
            watchedAddresses_[row].value = "??";
            storeModel_->setData(storeModel_->index(row, 1), QString::fromStdString(MedUtil::intToHex(watchedAddresses_[row].address)), Qt::EditRole);
            storeModel_->setData(storeModel_->index(row, 3), "??", Qt::EditRole);
            storeModel_->setData(storeModel_->index(row, 4), false, Qt::EditRole);
        }

        QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                                  Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
    } catch (...) {
        QMessageBox::warning(this, "Move", "Invalid offset value (should be a number)");
    }
}

void MainWindow::onDefaultEncodingTriggered(bool checked) {
    if (checked) {
        encoding_ = EncodingType::Default;
        QMetaObject::invokeMethod(worker_, "setEncoding", Qt::QueuedConnection, Q_ARG(EncodingType, encoding_));
        onRefreshRequested();
    }
}

void MainWindow::onBig5EncodingTriggered(bool checked) {
    if (checked) {
        encoding_ = EncodingType::Big5;
        QMetaObject::invokeMethod(worker_, "setEncoding", Qt::QueuedConnection, Q_ARG(EncodingType, encoding_));
        onRefreshRequested();
    }
}

void MainWindow::updateScanModel(const std::vector<ScanResult>& results) {
    isProgrammaticUpdate_ = true;
    scanModel_->removeRows(0, scanModel_->rowCount());
    size_t count = std::min(results.size(), (size_t)800);
    for (size_t i = 0; i < count; ++i) {
        const auto& res = results[i];
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::fromStdString(MedUtil::intToHex(res.address)));
        items << new QStandardItem(QString::fromStdString(MedUtil::scanTypeToString(res.type)));
        const SizedBytes& displayData = res.liveData.isEmpty() ? res.data : res.liveData;
        QStandardItem* valItem = new QStandardItem(QString::fromStdString(MemOperator::toString(displayData.getBytes(), displayData.getSize(), res.type, encoding_)));
        items << valItem;

        items[0]->setEditable(true);
        items[1]->setEditable(true);
        valItem->setEditable(true);

        scanModel_->appendRow(items);
    }
    if (foundLabel_) foundLabel_->setText(QString::number(results.size()));
    isProgrammaticUpdate_ = false;
}

void MainWindow::onNamedScanAddClicked() {
    if (!namedScanNameEdit_) return;
    QString name = namedScanNameEdit_->text().trimmed();
    if (name.isEmpty()) return;

    std::string sName = name.toStdString();
    namedScans_.addNewScan(sName, MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString()));

    if (namedScanCombo_) {
        if (namedScanCombo_->findText(name) == -1) {
            namedScanCombo_->addItem(name);
            namedScanCombo_->setCurrentText(name);
        }
    }
    namedScanNameEdit_->clear();
}

void MainWindow::onNamedScanDeleteClicked() {
    if (!namedScanCombo_) return;
    QString name = namedScanCombo_->currentText();
    if (name == med::NamedScans::DEFAULT_NAME) return;

    if (namedScans_.remove(name.toStdString())) {
        int index = namedScanCombo_->currentIndex();
        namedScanCombo_->setCurrentIndex(0);
        namedScanCombo_->removeItem(index);
    }
}

void MainWindow::onNamedScanComboBoxChanged(int) {
    if (!namedScanCombo_) return;
    QString name = namedScanCombo_->currentText();
    namedScans_.setActiveName(name.toStdString());

    // Update Scan Type Combo
    if (scanTypeCombo_) {
        scanTypeCombo_->setCurrentText(QString::fromStdString(MedUtil::scanTypeToString(namedScans_.getActiveType())));
    }

    updateScanModel(namedScans_.getActiveResults());
}

void MainWindow::onFileLoaded(const std::vector<WatchedAddress>& watched, const QString& notes) {
    watchedAddresses_ = watched;
    for (auto& wa : watchedAddresses_) {
        wa.value = "??";
        wa.locked = false; // Always unlock when loaded
    }
    if (notesEdit_) notesEdit_->setPlainText(notes);
    updateStoreModel();
    statusBar()->showMessage(QString("Loaded %1 addresses").arg(watchedAddresses_.size()));

    if (!currentFilename_.isEmpty()) {
        setWindowTitle(MAIN_TITLE + ": " + currentFilename_);
    }

    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection,
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onError(const QString& message) {
    // QMessageBox::warning(this, "Error", message);
    std::cout << message.toStdString() << std::endl;
    if (scanButton_) scanButton_->setEnabled(true);
    if (filterButton_) filterButton_->setEnabled(true);
}
