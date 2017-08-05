#ifndef MED_QT_HPP
#define MED_QT_HPP

#define SCAN_COL_ADDRESS 0
#define SCAN_COL_TYPE 1
#define SCAN_COL_VALUE 2

#define STORE_COL_DESCRIPTION 0
#define STORE_COL_ADDRESS 1
#define STORE_COL_TYPE 2
#define STORE_COL_VALUE 3
#define STORE_COL_LOCK 4

#include <QTreeWidgetItem>

#include "gui/TreeModel.hpp"
#include "gui/StoreTreeModel.hpp"
#include "gui/ComboBoxDelegate.hpp"
#include "gui/CheckBoxDelegate.hpp"
#include "med/med.hpp"

#include "med/med.hpp"

const QString MAIN_TITLE = "Med";

enum UiState { Idle, Editing };

class MainUi : public QObject {
  Q_OBJECT

public:
  MainUi(QApplication* app);

  Med med;
  std::thread* refreshThread;
  std::mutex scanUpdateMutex;
  std::mutex storeUpdateMutex; //TODO: Rename to storeUpdateMutex

  static void refresh(MainUi* mainUi);

  void refreshScanTreeView();
  void refreshStoreTreeView();

  // Accessors
  UiState getScanState();
  UiState getStoreState();
  void setScanState(UiState);
  void setStoreState(UiState);

  QWidget* mainWindow;
  QWidget* processSelector;
  QDialog* processDialog; // This is the processSelector container
  QTreeView* scanTreeView;
  QTreeView* storeTreeView;
  QLineEdit* selectedProcessLine;
  QComboBox* scanTypeCombo;
  QTreeWidget* processTreeWidget;
  StoreTreeModel * storeModel;

public slots:
  void onProcessItemDblClicked(QTreeWidgetItem* item, int column);

private slots:
  void onProcessClicked();
  void onScanClicked();
  void onFilterClicked();
  void onScanClearClicked();
  void onStoreClearClicked();
  void onScanAddClicked();
  void onScanAddAllClicked();
  void onStoreNewClicked();
  void onStoreDeleteClicked();
  void onStoreShiftClicked();
  void onStoreUnshiftClicked();
  void onStoreMoveClicked();
  void onSaveAsTriggered();
  void onSaveTriggered();
  void onOpenTriggered();
  void onReloadTriggered();
  void onQuitTriggered();
  void onScanTreeViewClicked(const QModelIndex &index);
  void onScanTreeViewDoubleClicked(const QModelIndex &index);
  void onStoreTreeViewDoubleClicked(const QModelIndex &index);
  void onStoreTreeViewClicked(const QModelIndex &index);
  void onScanTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
  void onStoreTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
  void onStoreHeaderClicked(int logicalIndex);

private:
  QApplication* app;

  UiState scanState;
  UiState storeState;
  QString filename;

  TreeModel* scanModel;

  void loadUiFiles();
  void loadProcessUi();
  void setupStatusBar();
  void setupScanTreeView();
  void setupStoreTreeView();
  void setupSignals();
  void setupUi();
  void updateNumberOfAddresses(QWidget* mainWindow);
  void setWindowTitle();
};

#endif
