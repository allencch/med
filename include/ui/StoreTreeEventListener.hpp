#include <QTreeView>
#include "ui/Ui.hpp"

class StoreTreeEventListener : public QObject {
  Q_OBJECT
public:
  StoreTreeEventListener(QTreeView* treeView, MedUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev);

private:
  MedUi* mainUi;
  QTreeView* treeView;
};
