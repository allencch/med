#include "ui/StoreTreeEventListener.hpp"
#include "med/MedCommon.hpp"

StoreTreeEventListener::StoreTreeEventListener(QTreeView* treeView, MedUi* mainUi) {
  this->mainUi = mainUi;
  this->treeView = treeView;
}


bool StoreTreeEventListener::eventFilter(QObject*, QEvent* ev) {
  if (ev->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);

    if (keyEvent->key() == Qt::Key_Escape) {
      if (mainUi->getStoreState() == UiState::Editing) {
        tryUnlock(mainUi->storeUpdateMutex);
        mainUi->setStoreState(UiState::Idle);
      }
    }

    if (keyEvent->key() == Qt::Key_F2) {
      QWidget* focused = mainUi->mainWindow->focusWidget()->parentWidget()->parentWidget();
      if (focused == treeView) {
        QModelIndex index = treeView->currentIndex();
        if (index.column() == STORE_COL_VALUE && mainUi->getStoreState() == UiState::Idle) {
          mainUi->storeUpdateMutex.lock();
          mainUi->setStoreState(UiState::Editing);
        }
      }
    }
  }
  return false;
}
