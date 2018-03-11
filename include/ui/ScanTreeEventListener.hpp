#include <QTreeView>
#include "ui/Ui.hpp"

class ScanTreeEventListener : public QObject {
  Q_OBJECT
public:
  ScanTreeEventListener(QTreeView* treeView, MedUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev);

private:
  MedUi* mainUi;
  QTreeView* treeView;
};
