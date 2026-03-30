#include <QTreeView>

#include "ui/Ui.hpp"

class ProcessDialogEventListener : public QObject {
  Q_OBJECT
public:
  explicit ProcessDialogEventListener(MedUi* mainUi);
protected:
  bool eventFilter(QObject* obj, QEvent* ev);
private:
  MedUi* mainUi;
};
