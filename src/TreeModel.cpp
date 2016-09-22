#include <QtWidgets>

#include "TreeItem.hpp"
#include "TreeModel.hpp"

TreeModel::TreeModel(QObject* parent) : QAbstractItemModel(parent) {
  QVector<QVariant> rootData;
  rootData << "Address" << "Type" << "Value";
  rootItem = new TreeItem(rootData);

  QVector<QVariant> data;
  data << "1" << "2" << "3";
  TreeItem* childrenItem = new TreeItem(data, rootItem);
  rootItem->appendChild(childrenItem);

  QVector<QVariant> data2;
  data2 << "4" << "5" << "6";
  TreeItem* child2 = new TreeItem(data2, rootItem);
  rootItem->appendChild(child2);
  //QStringList qstr = QStringList() << "hello" << "world";

  //setupModelData(qstr, rootItem);
}

TreeModel::~TreeModel() {
  delete rootItem;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  TreeItem* item = getItem(index);
  return item->data(index.column());
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role != Qt::EditRole)
    return false;

  TreeItem *item = getItem(index);
  bool result = item->setData(index.column(), value);

  if (result)
    emit dataChanged(index, index);

  return result;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return 0;
  return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const {
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section);
  return QVariant();
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role) {
  if (role != Qt::EditRole || orientation != Qt::Horizontal)
    return false;

  bool result = rootItem->setData(section, value);

  if (result)
    emit headerDataChanged(orientation, section, section);

  return result;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
  if(parent.isValid() && parent.column() != 0)
    return QModelIndex();

  TreeItem* parentItem = getItem(parent);
  TreeItem* childItem = parentItem->child(row);
  if(childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent) {
  bool success;
  beginInsertColumns(parent, position, position + columns -1);
  success = rootItem->insertColumns(position, columns);
  endInsertColumns();
  return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent) {
  TreeItem* parentItem = getItem(parent);
  bool success;
  beginInsertRows(parent, position, position + rows -1);
  success = parentItem->insertChildren(position, rows, rootItem->columnCount());
  endInsertRows();

  return success;
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent) {
  bool success;

  beginRemoveColumns(parent, position, position + columns - 1);
  success = rootItem->removeColumns(position, columns);
  endRemoveColumns();

  if (rootItem->columnCount() == 0)
    removeRows(0, rowCount());

  return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent) {
  TreeItem *parentItem = getItem(parent);
  bool success = true;

  beginRemoveRows(parent, position, position + rows - 1);
  success = parentItem->removeChildren(position, rows);
  endRemoveRows();

  return success;
}


QModelIndex TreeModel::parent(const QModelIndex &index) const {
  if(!index.isValid())
    return QModelIndex();

  TreeItem* childItem = getItem(index);
  TreeItem* parentItem = childItem->parent();
  if(parentItem == rootItem)
    return QModelIndex();
  return createIndex(parentItem->row(), 0, parentItem);
}


int TreeModel::rowCount(const QModelIndex &parent) const {
  TreeItem* parentItem = getItem(parent);
  return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex &parent) const {
  return rootItem->columnCount();
}

//My implementation
void TreeModel::appendRow(TreeItem* treeItem) {
  rootItem->appendChild(treeItem);
}

TreeItem* TreeModel::root() {
  return rootItem;
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem* parent) {
  //parent->appendChild(new TreeItem(lines), parent);
}

TreeItem* TreeModel::getItem(const QModelIndex &index) const {
  if(index.isValid()) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if(item)
      return item;
  }
  return rootItem;
}
