#ifndef MED_WORKER_HPP
#define MED_WORKER_HPP

#include <QObject>
#include <QTimer>
#include <vector>
#include "med/MemScanner.hpp"
#include "med/Process.hpp"

// Represents a watched address in the "Store" (bottom table)
struct WatchedAddress {
    std::string description;
    Address address;
    ScanType type;
    std::string value; // Current value as string
    bool locked = false;
    std::string lockValue;
};

Q_DECLARE_METATYPE(std::vector<ScanResult>)
Q_DECLARE_METATYPE(std::vector<WatchedAddress>)
Q_DECLARE_METATYPE(ScanType)
Q_DECLARE_METATYPE(ScanParser::OpType)
Q_DECLARE_METATYPE(pid_t)

class MedWorker : public QObject {
    Q_OBJECT

public:
    explicit MedWorker(QObject* parent = nullptr);
    ~MedWorker();

signals:
    void scanCompleted(const std::vector<ScanResult>& results);
    void filterCompleted(const std::vector<ScanResult>& results);
    void watchedValuesRefreshed(const std::vector<WatchedAddress>& watched);
    void errorOccurred(const QString& message);
    void processListReady(const std::vector<Process>& processes);

public slots:
    void setPid(pid_t pid);
    void startScan(const QString& value, ScanType type, ScanParser::OpType op, bool fastScan, const std::vector<int>& lastDigits);
    void startFilter(const std::vector<ScanResult>& currentResults, const QString& value, ScanType type, ScanParser::OpType op);
    
    void updateWatchedAddresses(const std::vector<WatchedAddress>& watched);
    void writeMemory(Address addr, const QString& value, ScanType type);
    
    void requestProcessList();
    void refreshWatchedValues();

private:
    pid_t pid_ = 0;
    std::unique_ptr<MemScanner> scanner_;
    std::vector<WatchedAddress> watched_;
    QTimer* refreshTimer_;
    
    void performLocks();
};

#endif
