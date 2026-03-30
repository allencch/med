#include "ui/ProcessEventListener.hpp"

ProcessDialogEventListener::ProcessDialogEventListener(MedUi* mainUi) {
  this->mainUi = mainUi;
}

bool ProcessDialogEventListener::eventFilter(QObject* obj, QEvent* ev) {
  if(obj == mainUi->processTreeWidget && ev->type() == QEvent::KeyRelease) {
    if(static_cast<QKeyEvent*>(ev)->key() == Qt::Key_Return) { //Use Return instead of Enter
      mainUi->onProcessItemDblClicked(mainUi->processTreeWidget->currentItem(), 0); //Just use the first column
    }
  }
  return false;
}
