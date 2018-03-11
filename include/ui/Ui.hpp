#ifndef MED_UI_HPP
#define MED_UI_HPP

#define SCAN_COL_ADDRESS 0
#define SCAN_COL_TYPE 1
#define SCAN_COL_VALUE 2

#define STORE_COL_DESCRIPTION 0
#define STORE_COL_ADDRESS 1
#define STORE_COL_TYPE 2
#define STORE_COL_VALUE 3
#define STORE_COL_LOCK 4

#define SCAN_ADDRESS_VISIBLE_SIZE 400

#include <QTreeWidgetItem>
#include <QStatusBar>
#include <QPlainTextEdit>
#include <QComboBox>

#include "ui/TreeModel.hpp"
#include "mem/MemEd.hpp"


const QString MAIN_TITLE = "Med UI";

enum UiState { Idle, Editing };

class EncodingManager; // Forward declaration, because of recursive include

class MedUi : public QObject {
  Q_OBJECT

public:
  explicit MedUi(QApplication* app);
  ~MedUi();

  QWidget* mainWindow;
  QWidget* processSelector;
  QDialog* processDialog; // This is the processSelector container
  QTreeWidget* processTreeWidget;
  QLineEdit* selectedProcessLine;
  QStatusBar* statusBar;
  QComboBox* scanTypeCombo;
  QTreeView* scanTreeView;
  QTreeView* storeTreeView;

  EncodingManager* encodingManager;
  MemEd* med;

public slots:
  void onProcessItemDblClicked(QTreeWidgetItem* item, int column);

private slots:
  void onProcessClicked();
  void onScanClicked();
  void onFilterClicked();

  void onScanTreeViewClicked(const QModelIndex &index);
  void onScanTreeViewDoubleClicked(const QModelIndex &index);
  void onScanTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());

private:
  void loadUiFiles();
  void loadProcessUi();
  void setupStatusBar();
  void setupScanTreeView();
  void setupSignals();
  void setupUi();
  void updateNumberOfAddresses();

  QApplication* app;
  TreeModel* scanModel;
};


#endif
