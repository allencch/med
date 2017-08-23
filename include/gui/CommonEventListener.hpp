#ifndef COMMON_EVENT_LISTENER_HPP
#define COMMON_EVENT_LISTENER_HPP

#include <QTreeView>

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

class ScanTreeEventListener : public QObject {
  Q_OBJECT
public:
  ScanTreeEventListener(QTreeView* treeView, MainUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev);

private:
  MainUi* mainUi;
  QTreeView* treeView;
};

class StoreTreeEventListener : public QObject {
  Q_OBJECT
public:
  StoreTreeEventListener(QTreeView* treeView, MainUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev);

private:
  MainUi* mainUi;
  QTreeView* treeView;
};

#endif
