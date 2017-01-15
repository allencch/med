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
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
  void addScan();
  void refresh();
};

#endif
