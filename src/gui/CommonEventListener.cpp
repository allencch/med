#include "gui/CommonEventListener.hpp"

/*************\
 ProcessDialog
\*************/

ProcessDialogEventListener::ProcessDialogEventListener(MainUi* mainUi) {
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

/**************\
 Scan tree
\**************/

ScanTreeEventListener::ScanTreeEventListener(QTreeView* treeView, MainUi* mainUi) {
  this->mainUi = mainUi;
  this->treeView = treeView;
}


bool ScanTreeEventListener::eventFilter(QObject*, QEvent* ev) {
  if (ev->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);

    if (keyEvent->key() == Qt::Key_Escape) {
      if (mainUi->getScanState() == UiState::Editing) {
        tryUnlock(mainUi->scanUpdateMutex);
        mainUi->setScanState(UiState::Idle);
      }
    }

    if (keyEvent->key() == Qt::Key_F2) {
      QWidget* focused = mainUi->mainWindow->focusWidget()->parentWidget()->parentWidget(); // When the TreeView item value is selected, the TreeView will be the grandparent of the editing
      if (focused == treeView) {
        QModelIndex index = treeView->currentIndex();
        if (index.column() == SCAN_COL_VALUE && mainUi->getScanState() == UiState::Idle) {
          mainUi->scanUpdateMutex.lock();
          mainUi->setScanState(UiState::Editing);
        }
      }
    }
  }
  return false;
}


/**************\
 Store tree
\**************/

StoreTreeEventListener::StoreTreeEventListener(QTreeView* treeView, MainUi* mainUi) {
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
