#include <QVBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QStatusBar>
#include <QtUiTools/QUiLoader>
#include <QFile>

#include "ui/MainWindow.hpp"
#include "med/MedCommon.hpp"
#include "med/MemOperator.hpp"

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
    QFile file("ui/main-qt.ui");
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open UI file");
        return;
    }
    QWidget* centralWidget = loader.load(&file, this);
    file.close();
    
    if (!centralWidget) {
        QMessageBox::critical(this, "Error", "Could not load UI");
        return;
    }

    setCentralWidget(centralWidget);
    setWindowTitle("Med Rewrite");

    // Finding components (names are based on old ui files)
    scanTreeView_ = centralWidget->findChild<QTreeView*>("scanTreeView");
    storeTreeView_ = centralWidget->findChild<QTreeView*>("storeTreeView");
    scanValueEdit_ = centralWidget->findChild<QLineEdit*>("scanValueEdit");
    scanTypeCombo_ = centralWidget->findChild<QComboBox*>("scanTypeCombo");
    scanOpCombo_ = centralWidget->findChild<QComboBox*>("scanOpCombo");
    
    QPushButton* scanButton = centralWidget->findChild<QPushButton*>("scanButton");
    QPushButton* filterButton = centralWidget->findChild<QPushButton*>("filterButton");
    QPushButton* processButton = centralWidget->findChild<QPushButton*>("processButton");
    QPushButton* addButton = centralWidget->findChild<QPushButton*>("addButton");

    if (scanButton) connect(scanButton, &QPushButton::clicked, this, &MainWindow::onScanClicked);
    if (filterButton) connect(filterButton, &QPushButton::clicked, this, &MainWindow::onFilterClicked);
    if (processButton) connect(processButton, &QPushButton::clicked, this, &MainWindow::onSelectProcessClicked);
    if (addButton) connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddToStoreClicked);

    // Setup Models
    scanModel_ = new QStandardItemModel(this);
    scanModel_->setHorizontalHeaderLabels({"Address", "Type", "Value"});
    if (scanTreeView_) scanTreeView_->setModel(scanModel_);

    storeModel_ = new QStandardItemModel(this);
    storeModel_->setHorizontalHeaderLabels({"Description", "Address", "Type", "Value", "Lock"});
    if (storeTreeView_) storeTreeView_->setModel(storeModel_);
}

void MainWindow::connectSignals() {
    // UI to Worker (Cross-thread via QueuedConnection automatically by Qt)
    connect(this, SIGNAL(scanRequested(QString, ScanType, ScanParser::OpType, bool, std::vector<int>)), 
            worker_, SLOT(startScan(QString, ScanType, ScanParser::OpType, bool, std::vector<int>)));
    
    // Worker to UI
    connect(worker_, &MedWorker::scanCompleted, this, &MainWindow::onScanCompleted);
    connect(worker_, &MedWorker::filterCompleted, this, &MainWindow::onFilterCompleted);
    connect(worker_, &MedWorker::watchedValuesRefreshed, this, &MainWindow::onWatchedValuesRefreshed);
    connect(worker_, &MedWorker::processListReady, this, &MainWindow::onProcessListReady);
    connect(worker_, &MedWorker::errorOccurred, this, &MainWindow::onError);
}

// These should be signals in the class definition for cleaner code,
// but for now I'll use direct meta-calls or just fix the header later.
// Let's fix the header to include the necessary signals.

void MainWindow::onScanClicked() {
    if (currentPid_ == 0) {
        QMessageBox::warning(this, "Process", "Select a process first");
        return;
    }
    
    QString val = scanValueEdit_->text();
    ScanType type = MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString());
    ScanParser::OpType op = ScanParser::getOpType(scanOpCombo_->currentText().toStdString());
    
    // Using QMetaObject::invokeMethod for a simple way without redeclaring signals now
    QMetaObject::invokeMethod(worker_, "startScan", Qt::QueuedConnection,
                              Q_ARG(QString, val),
                              Q_ARG(ScanType, type),
                              Q_ARG(ScanParser::OpType, op),
                              Q_ARG(bool, false),
                              Q_ARG(std::vector<int>, std::vector<int>()));
    
    statusBar()->showMessage("Scanning...");
}

void MainWindow::onFilterClicked() {
    QString val = scanValueEdit_->text();
    ScanType type = MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString());
    ScanParser::OpType op = ScanParser::getOpType(scanOpCombo_->currentText().toStdString());

    QMetaObject::invokeMethod(worker_, "startFilter", Qt::QueuedConnection,
                              Q_ARG(std::vector<ScanResult>, lastScanResults_),
                              Q_ARG(QString, val),
                              Q_ARG(ScanType, type),
                              Q_ARG(ScanParser::OpType, op));
    
    statusBar()->showMessage("Filtering...");
}

void MainWindow::onSelectProcessClicked() {
    worker_->requestProcessList();
}

void MainWindow::onScanCompleted(const std::vector<ScanResult>& results) {
    lastScanResults_ = results;
    scanModel_->removeRows(0, scanModel_->rowCount());
    
    for (const auto& res : results) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::fromStdString(MedUtil::intToHex(res.address)));
        items << new QStandardItem(QString::fromStdString(MedUtil::scanTypeToString(res.type)));
        items << new QStandardItem(QString::fromStdString(MemOperator::toString(res.data.getBytes(), res.type)));
        scanModel_->appendRow(items);
    }
    statusBar()->showMessage(QString("Found %1 addresses").arg(results.size()));
}

void MainWindow::onFilterCompleted(const std::vector<ScanResult>& results) {
    onScanCompleted(results);
}

void MainWindow::onWatchedValuesRefreshed(const std::vector<WatchedAddress>& watched) {
    watchedAddresses_ = watched;
    for (int i = 0; i < (int)watched.size(); ++i) {
        storeModel_->setData(storeModel_->index(i, 3), QString::fromStdString(watched[i].value));
    }
}

void MainWindow::onProcessListReady(const std::vector<Process>& processes) {
    QStringList items;
    for (const auto& p : processes) {
        items << QString("%1: %2").arg(p.getPid()).arg(QString::fromStdString(p.getCmdline()));
    }
    
    bool ok;
    QString item = QInputDialog::getItem(this, "Select Process", "Process:", items, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        pid_t pid = item.split(":").at(0).toInt();
        currentPid_ = pid;
        QMetaObject::invokeMethod(worker_, "setPid", Qt::QueuedConnection, Q_ARG(pid_t, pid));
        statusBar()->showMessage(QString("Attached to PID %1").arg(pid));
    }
}

void MainWindow::onAddToStoreClicked() {
    // Add selected from scanTreeView to store
    auto index = scanTreeView_->currentIndex();
    if (!index.isValid()) return;
    
    int row = index.row();
    const auto& res = lastScanResults_[row];
    
    WatchedAddress wa;
    wa.description = "New Address";
    wa.address = res.address;
    wa.type = res.type;
    wa.value = MemOperator::toString(res.data.getBytes(), res.type);
    
    watchedAddresses_.push_back(wa);
    
    QList<QStandardItem*> items;
    items << new QStandardItem(QString::fromStdString(wa.description));
    items << new QStandardItem(QString::fromStdString(MedUtil::intToHex(wa.address)));
    items << new QStandardItem(QString::fromStdString(MedUtil::scanTypeToString(wa.type)));
    items << new QStandardItem(QString::fromStdString(wa.value));
    items << new QStandardItem("No");
    storeModel_->appendRow(items);
    
    QMetaObject::invokeMethod(worker_, "updateWatchedAddresses", Qt::QueuedConnection, 
                              Q_ARG(std::vector<WatchedAddress>, watchedAddresses_));
}

void MainWindow::onError(const QString& message) {
    QMessageBox::warning(this, "Error", message);
}
