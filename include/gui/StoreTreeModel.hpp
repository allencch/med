#ifndef STORE_TREEMODEL_H
#define STORE_TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "gui/TreeItem.hpp"
#include "gui/TreeModel.hpp"
#include "med/med.hpp"

class MainUi;

class StoreTreeModel : public TreeModel {
  Q_OBJECT
public:
  StoreTreeModel(MainUi* mainUi, QObject* parent = 0);
  ~StoreTreeModel();
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
  void refresh();
  void refreshValues(); //Refresh values only
  void addRow();

  void sortByDescription();
  void sortByAddress();
  void empty();

protected:
  void setValue(int row, const QVariant &value);
  void setType(int row, const QVariant &value, const QModelIndex &index);
  void setAddress(int row, const QVariant &value, const QModelIndex &index);
};

#endif
