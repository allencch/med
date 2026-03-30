#include <QDebug>
#include "ui/MedWorker.hpp"
#include "med/MedException.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

MedWorker::MedWorker(QObject* parent) : QObject(parent) {
    refreshTimer_ = new QTimer(this);
    connect(refreshTimer_, &QTimer::timeout, this, &MedWorker::refreshWatchedValues);
    refreshTimer_->start(800);
}

MedWorker::~MedWorker() {}

void MedWorker::setPid(pid_t pid) {
    pid_ = pid;
    scanner_ = std::make_unique<MemScanner>(pid_);
}

void MedWorker::startScan(const QString& value, ScanType type, ScanParser::OpType op, bool fastScan, const std::vector<int>& lastDigits) {
    if (!scanner_) {
        emit errorOccurred("No process selected");
        return;
    }

    try {
        ScanParams params;
        params.type = type;
        params.op = op;
        params.fastScan = fastScan;
        params.lastDigits = lastDigits;
        params.operands = ScanParser::valueToOperands(value.toStdString(), type, op);

        auto results = scanner_->scan(params);
        emit scanCompleted(results);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void MedWorker::startFilter(const std::vector<ScanResult>& currentResults, const QString& value, ScanType type, ScanParser::OpType op) {
    if (!scanner_) return;

    try {
        ScanParams params;
        params.type = type;
        params.op = op;
        params.operands = ScanParser::valueToOperands(value.toStdString(), type, op);

        auto results = scanner_->filter(currentResults, params);
        emit filterCompleted(results);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void MedWorker::updateWatchedAddresses(const std::vector<WatchedAddress>& watched) {
    watched_ = watched;
}

void MedWorker::writeMemory(Address addr, const QString& value, ScanType type) {
    if (pid_ == 0) return;
    try {
        MemIO memio(pid_);
        size_t size = MedUtil::scanTypeToSize(type);
        SizedBytes sb = SizedBytes::create(size);
        MedUtil::stringToMemory(value.toStdString(), type, sb.getBytes());
        memio.write(addr, sb);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void MedWorker::requestProcessList() {
    try {
        auto processes = Process::listAll();
        emit processListReady(processes);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void MedWorker::refreshWatchedValues() {
    if (pid_ == 0 || watched_.empty()) return;

    performLocks();

    try {
        MemIO memio(pid_);
        for (auto& w : watched_) {
            size_t size = MedUtil::scanTypeToSize(w.type);
            try {
                SizedBytes data = memio.read(w.address, size);
                w.value = MemOperator::toString(data.getBytes(), w.type);
            } catch (...) {
                w.value = "??";
            }
        }
        emit watchedValuesRefreshed(watched_);
    } catch (...) {}
}

void MedWorker::performLocks() {
    if (pid_ == 0) return;
    MemIO memio(pid_);
    for (const auto& w : watched_) {
        if (w.locked) {
            try {
                size_t size = MedUtil::scanTypeToSize(w.type);
                SizedBytes sb = SizedBytes::create(size);
                MedUtil::stringToMemory(w.lockValue, w.type, sb.getBytes());
                memio.write(w.address, sb);
            } catch (...) {}
        }
    }
}
