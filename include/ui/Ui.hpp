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

#include <QTreeWidgetItem>
#include <QStatusBar>
#include <QPlainTextEdit>

#include "mem/MemEd.hpp"


const QString MAIN_TITLE = "Med UI";

enum UiState { Idle, Editing };

class MedUi : public QObject {
  Q_OBJECT

public:
  explicit MedUi(QApplication* app);

  MemEd med;
private:
  QApplication* app;
  QWidget* mainWindow;
  void loadUiFiles();
  void setupUi();
};


#endif
