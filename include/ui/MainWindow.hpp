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
#include "med/NamedScans.hpp"
#include "ui/MedWorker.hpp"
#include "ui/ProcessDialog.hpp"

class MemEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    pid_t getPid() const { return currentPid_; }
    MedWorker* getWorker() const { return worker_; }
    EncodingType getEncoding() const { return encoding_; }

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
    void onAutoRefreshTriggered(bool checked);
    void onForceResumeTriggered(bool checked);
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
    void onStoreClearTriggered();
    
    void onPrevAddressClicked();
    void onNextAddressClicked();
    void onStoreShiftClicked();
    void onStoreUnshiftClicked();
    void onMoveAddressClicked();
    void onStoreHeaderClicked(int logicalIndex);
    void onDefaultEncodingTriggered(bool checked);
    void onBig5EncodingTriggered(bool checked);
    
    void onNamedScanAddClicked();
    void onNamedScanDeleteClicked();
    void onNamedScanComboBoxChanged(int index);
    
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
    void updateScanModel(const std::vector<ScanResult>& results);
    void addWatchedAddress(const WatchedAddress& wa);
    void updateStoreModel();
    
    // UI elements from .ui
    QTreeView* scanTreeView_;
    QTreeView* storeTreeView_;
    QLineEdit* scanValueEdit_;
    QLineEdit* lastDigitEdit_;
    QComboBox* scanTypeCombo_;
    QLabel* foundLabel_;
    QPlainTextEdit* notesEdit_;
    QLineEdit* selectedProcessEdit_;
    QLineEdit* shiftFromEdit_;
    QLineEdit* shiftToEdit_;
    
    QComboBox* namedScanCombo_;
    QLineEdit* namedScanNameEdit_;
    QPushButton* namedScanAddBtn_;
    QPushButton* namedScanDeleteBtn_;
    QPushButton* scanButton_;
    QPushButton* filterButton_;
    
    QStandardItemModel* scanModel_;
    QStandardItemModel* storeModel_;
    
    ProcessDialog* processDialog_ = nullptr;
    MemEditor* memEditor_ = nullptr;
    QString currentFilename_;
    
    med::NamedScans namedScans_;
    std::vector<WatchedAddress> watchedAddresses_;

    // Threading
    QThread workerThread_;
    MedWorker* worker_;
    
    pid_t currentPid_ = 0;
    bool fastScan_ = true;
    bool autoRefresh_ = true;
    bool forceResume_ = false;
    bool isProgrammaticUpdate_ = false;
    EncodingType encoding_ = EncodingType::Default;
};

#endif
