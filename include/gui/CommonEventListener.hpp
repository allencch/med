#include <QObject>

#include "gui/med-qt.hpp"

class ProcessDialogEventListener : public QObject {
  Q_OBJECT
public:
  ProcessDialogEventListener(MainUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev);
private:
  MainUi* mainUi;
};


class MainWindowEventListener : public QObject {
  Q_OBJECT
public:
  MainWindowEventListener(MainUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev) ;

private:
  MainUi* mainUi;
};
