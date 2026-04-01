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
#include <QLabel>
#include <QPlainTextEdit>
#include "ui/MedWorker.hpp"
#include "ui/ProcessDialog.hpp"

class MemEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    pid_t getPid() const { return currentPid_; }

private slots:
    // UI actions
    void onSelectProcessClicked();
    void onProcessSelected(pid_t pid, const QString& name);
    void onScanClicked();
    void onFilterClicked();
    void onAddToStoreClicked();
    void onAddAllToStoreClicked();
    void onScanClearClicked();
    
    void onScanTreeViewDoubleClicked(const QModelIndex& index);
    void onScanDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void onStoreTreeViewDoubleClicked(const QModelIndex& index);
    void onStoreDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    void onPauseClicked(bool checked);
    void onCanResumeTriggered(bool checked);
    void onFastScanTriggered(bool checked);
    
    void onOpenTriggered();
    void onSaveTriggered();
    void onSaveAsTriggered();
    void onReloadTriggered();
    void onShowNotesTriggered(bool checked);
    void onMemEditorTriggered();
    
    void onNewAddressTriggered();
    void onDeleteAddressTriggered();
    void onUnlockAllTriggered();
    
    void onPrevAddressClicked();
    void onNextAddressClicked();
    
    // Worker responses
    void onScanCompleted(const std::vector<ScanResult>& results);
    void onFilterCompleted(const std::vector<ScanResult>& results);
    void onWatchedValuesRefreshed(const std::vector<WatchedAddress>& watched);
    void onRefreshRequested();
    void onProcessListReady(const std::vector<Process>& processes);
    void onFileLoaded(const std::vector<WatchedAddress>& watched, const QString& notes);
    void onError(const QString& message);

private:
    void setupUi();
    void connectSignals();
    
    // UI elements from .ui
    QTreeView* scanTreeView_;
    QTreeView* storeTreeView_;
    QLineEdit* scanValueEdit_;
    QComboBox* scanTypeCombo_;
    QLabel* foundLabel_;
    QPlainTextEdit* notesEdit_;
    QLineEdit* selectedProcessEdit_;
    
    QStandardItemModel* scanModel_;
    QStandardItemModel* storeModel_;
    
    ProcessDialog* processDialog_ = nullptr;
    MemEditor* memEditor_ = nullptr;
    QString currentFilename_;
    
    std::vector<ScanResult> lastScanResults_;
    std::vector<WatchedAddress> watchedAddresses_;

    // Threading
    QThread workerThread_;
    MedWorker* worker_;
    
    pid_t currentPid_ = 0;
    bool fastScan_ = true;
    bool autoRefresh_ = true;
};

#endif
