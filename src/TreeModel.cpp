#include <QtWidgets>
#include <iostream>
#include <cstdio>
#include "med-qt.hpp"
#include "TreeItem.hpp"
#include "TreeModel.hpp"

using namespace std;

TreeModel::TreeModel(Med* med, QObject* parent) : QAbstractItemModel(parent) {
  QVector<QVariant> rootData;
  rootData << "Address" << "Type" << "Value";
  rootItem = new TreeItem(rootData);

  this->med = med;

  /*QVector<QVariant> data;
  data << "1" << "int8" << "3";
  TreeItem* childrenItem = new TreeItem(data, rootItem);
  rootItem->appendChild(childrenItem);

  QVector<QVariant> data2;
  data2 << "4" << "int8" << "6";
  TreeItem* child2 = new TreeItem(data2, rootItem);
  rootItem->appendChild(child2);//*/
  //QStringList qstr = QStringList() << "hello" << "world";

  //setupModelData(qstr, rootItem);
}

TreeModel::~TreeModel() {
  delete rootItem;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  //if(role == Qt::CheckStateRole && index.column() == 0) {
    //return static_cast<int>(Qt::Checked);
  //}

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  TreeItem* item = getItem(index);
  return item->data(index.column());
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.column() == SCAN_COL_ADDRESS)
    return false;

  if (role != Qt::EditRole)
    return false;

  //Update the med
  //Update, split this out
  int row = index.row();
  if(index.column() == SCAN_COL_VALUE) { //value
    try {
      med->setValueByAddress(med->scanAddresses[row].address,
                             value.toString().toStdString(),
                             med->scanAddresses[row].getScanType());
    } catch(MedException &e) {
      cerr << "editScanValue: " << e.what() << endl;
    }
  }
  else if (index.column() == SCAN_COL_TYPE) {
    try {
      med->scanAddresses[row].setScanType(value.toString().toStdString());
      string value2 = med->getValueByAddress(med->scanAddresses[row].address,
                                             value.toString().toStdString());
      QVariant valueToSet = QString::fromStdString(value2);

      TreeItem *item = getItem(index);
      item->setData(SCAN_COL_VALUE, valueToSet); //Update the target value
    } catch(MedException &e) {
      cerr << "editScanType: " << e.what() << endl;
    }
  }

  TreeItem *item = getItem(index);
  bool result = item->setData(index.column(), value); //Update the cell

  if (result)
    emit dataChanged(index, index);

  return result;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return 0;

  Qt::ItemFlags flags = Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  //if(index.column() == SCAN_COL_ADDRESS)
  //  flags |= Qt::ItemIsUserCheckable;
  return flags;
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
  beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);  //This is important, else the View will not be updated when the Model is modified by the signal.
  rootItem->appendChild(treeItem);
  endInsertRows();
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

void TreeModel::clearAll() {
  removeRows(0, rowCount());
}

void TreeModel::addScan(string scanType) {
  this->clearAll();
  for(int i=0;i<med->scanAddresses.size();i++) {
    string address = med->getScanAddressByIndex(i);
    string value = med->getScanAddressValueByIndex(i, scanType);
    QVector<QVariant> data;
    data << address.c_str() << scanType.c_str() << value.c_str();
    TreeItem* childItem = new TreeItem(data, this->root());
    this->appendRow(childItem);
  }
}
