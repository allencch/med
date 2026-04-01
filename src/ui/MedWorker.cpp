#include <QDebug>
#include <fstream>
#include <json/json.h>
#include "ui/MedWorker.hpp"
#include "med/MedException.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

MedWorker::MedWorker(QObject* parent) : QObject(parent) {
    refreshTimer_ = new QTimer(this);
    connect(refreshTimer_, &QTimer::timeout, this, &MedWorker::onTick);
    refreshTimer_->start(800);
}

MedWorker::~MedWorker() {}

void MedWorker::onTick() {
    emit refreshRequested();
    refreshWatchedValues();
}

void MedWorker::setPid(pid_t pid) {
    pid_ = pid;
    scanner_ = std::make_unique<MemScanner>(pid_);
    if (scopeStart_ || scopeEnd_) {
        scanner_->setScope(scopeStart_, scopeEnd_);
    }
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

        if (!isProcessPaused_ && canResume_) {
            Process(pid_, "").resume();
        }
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

        if (!isProcessPaused_ && canResume_) {
            Process(pid_, "").resume();
        }
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

void MedWorker::refreshScanResults(const std::vector<ScanResult>& current) {
    if (pid_ == 0 || current.empty()) return;

    std::vector<ScanResult> updated = current;
    MemIO memio(pid_);
    for (auto& res : updated) {
        try {
            size_t size = MedUtil::scanTypeToSize(res.type);
            res.data = memio.read(res.address, size);
        } catch (...) {}
    }
    emit scanCompleted(updated);
}

void MedWorker::requestMemory(Address addr, size_t size) {
    if (pid_ == 0) return;
    try {
        MemIO memio(pid_);
        SizedBytes data = memio.read(addr, size);
        emit memoryReady(addr, data);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void MedWorker::setProcessPaused(bool paused) {
    if (pid_ == 0) return;
    isProcessPaused_ = paused;
    if (isProcessPaused_) {
        Process(pid_, "").stop();
    } else {
        Process(pid_, "").resume();
    }
}

void MedWorker::setCanResume(bool canResume) {
    canResume_ = canResume;
}

void MedWorker::setScopeStart(Address start) {
    scopeStart_ = start;
    if (scanner_) scanner_->setScope(scopeStart_, scopeEnd_);
}

void MedWorker::setScopeEnd(Address end) {
    scopeEnd_ = end;
    if (scanner_) scanner_->setScope(scopeStart_, scopeEnd_);
}

void MedWorker::saveFile(const QString& filename, const QString& notes) {
    Json::Value root;
    Json::Value addresses(Json::arrayValue);
    
    for (const auto& w : watched_) {
        Json::Value item;
        item["description"] = w.description;
        item["address"] = MedUtil::intToHex(w.address);
        item["type"] = MedUtil::scanTypeToString(w.type);
        item["lock"] = w.locked;
        item["lockValue"] = w.lockValue;
        addresses.append(item);
    }
    root["addresses"] = addresses;
    root["notes"] = notes.toStdString();

    std::ofstream ofs(filename.toStdString());
    if (!ofs.is_open()) {
        emit errorOccurred("Failed to open file for saving: " + filename);
        return;
    }
    ofs << root << std::endl;
    ofs.close();
}

void MedWorker::loadFile(const QString& filename) {
    std::ifstream ifs(filename.toStdString());
    if (!ifs.is_open()) {
        emit errorOccurred("Failed to open file for loading: " + filename);
        return;
    }

    Json::Value root;
    ifs >> root;
    ifs.close();

    std::vector<WatchedAddress> newWatched;
    auto& addresses = root["addresses"];
    for (int i = 0; i < (int)addresses.size(); ++i) {
        WatchedAddress w;
        w.description = addresses[i]["description"].asString();
        w.address = MedUtil::hexToInt(addresses[i]["address"].asString());
        w.type = MedUtil::stringToScanType(addresses[i]["type"].asString());
        w.locked = addresses[i]["lock"].asBool();
        w.lockValue = addresses[i].get("lockValue", "").asString();
        newWatched.push_back(w);
    }
    
    watched_ = newWatched;
    QString notes = QString::fromStdString(root.get("notes", "").asString());
    emit fileLoaded(watched_, notes);
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
