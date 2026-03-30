#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QThread>
#include <QTreeView>
#include <QStandardItemModel>
#include <QtUiTools/QUiLoader>
#include <QFile>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "ui/MedWorker.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // UI actions
    void onSelectProcessClicked();
    void onScanClicked();
    void onFilterClicked();
    void onAddToStoreClicked();
    
    // Worker responses
    void onScanCompleted(const std::vector<ScanResult>& results);
    void onFilterCompleted(const std::vector<ScanResult>& results);
    void onWatchedValuesRefreshed(const std::vector<WatchedAddress>& watched);
    void onProcessListReady(const std::vector<Process>& processes);
    void onError(const QString& message);

private:
    void setupUi();
    void connectSignals();
    
    // UI elements from .ui
    QTreeView* scanTreeView_;
    QTreeView* storeTreeView_;
    QLineEdit* scanValueEdit_;
    QComboBox* scanTypeCombo_;
    QComboBox* scanOpCombo_;
    
    QStandardItemModel* scanModel_;
    QStandardItemModel* storeModel_;
    
    std::vector<ScanResult> lastScanResults_;
    std::vector<WatchedAddress> watchedAddresses_;

    // Threading
    QThread workerThread_;
    MedWorker* worker_;
    
    pid_t currentPid_ = 0;
};

#endif
