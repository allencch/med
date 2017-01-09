#ifndef STORE_TREEMODEL_H
#define STORE_TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "TreeItem.hpp"
#include "TreeModel.hpp"
#include "med.hpp"

class StoreTreeModel : public TreeModel {
  Q_OBJECT
public:
  StoreTreeModel(Med* med, QObject* parent = 0);
  ~StoreTreeModel();
};

#endif
