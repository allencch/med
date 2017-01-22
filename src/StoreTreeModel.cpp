#include <QtWidgets>
#include <iostream>
#include <cstdio>
#include "med-qt.hpp"
#include "TreeItem.hpp"
#include "TreeModel.hpp"
#include "StoreTreeModel.hpp"

using namespace std;

StoreTreeModel::StoreTreeModel(Med* med, QObject* parent) : TreeModel(med, parent) {
  QVector<QVariant> rootData;
  rootData << "Description +" << "Address +" << "Type" << "Value" << "Lock";
  rootItem = new TreeItem(rootData);
  this->med = med;
}

StoreTreeModel::~StoreTreeModel() {
  delete rootItem;
}

QVariant StoreTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  TreeItem* item = getItem(index);
  //cout << Qt::CheckStateRole << endl;
  //cout << role << endl;
  if (role == Qt::CheckStateRole && index.column() == ADDRESS_COL_LOCK) {
    return item->data(index.column()).toBool() ? Qt::Checked : Qt::Unchecked;
  }

  return item->data(index.column());
}

bool StoreTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role != Qt::EditRole)
    return false;

  int row = index.row();
  if(index.column() == ADDRESS_COL_VALUE) {
    try {
      //TODO: Refactor
      med->setValueByAddress(med->addresses[row]->address,
                             value.toString().toStdString(),
                             med->addresses[row]->getScanType());
    } catch(MedException &e) {
      cerr << "editStoreValue: " << e.what() << endl;
    }
  }
  else if (index.column() == ADDRESS_COL_TYPE) {
    try {
      //TODO: refactor
      med->addresses[row]->setScanType(value.toString().toStdString());
      string value2 = med->getValueByAddress(med->addresses[row]->address,
                                             value.toString().toStdString());
      QVariant valueToSet = QString::fromStdString(value2);

      TreeItem *item = getItem(index);
      item->setData(ADDRESS_COL_VALUE, valueToSet); //Update the target value
    } catch(MedException &e) {
      cerr << "editStoreType: " << e.what() << endl;
    }
  }
  else if (index.column() == ADDRESS_COL_ADDRESS) {
    try {
      string value2 = med->setStoreAddressByIndex(row, value.toString().toStdString());
      QVariant valueToSet = QString::fromStdString(value2);

      TreeItem *item = getItem(index);
      item->setData(ADDRESS_COL_VALUE, valueToSet); //Update the target value
     } catch(MedException &e) {
      cerr << "editStoreAddress: " << e.what() << endl;
    }
  }
  else if (index.column() == ADDRESS_COL_LOCK) {
    med->setStoreLockByIndex(row, value.toBool());
  }
  else if (index.column() == ADDRESS_COL_DESCRIPTION) {
    med->setStoreDescriptionByIndex(row, value.toString().toStdString());
  }

  TreeItem *item = getItem(index);
  bool result = item->setData(index.column(), value); //Update the cell

  if (result)
    emit dataChanged(index, index);

  return result;
}

Qt::ItemFlags StoreTreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return 0;
  Qt::ItemFlags flags = Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  if (index.column() == ADDRESS_COL_LOCK) {
    flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

void StoreTreeModel::refreshValues() {
  for(int i=0; i < med->addresses.size(); i++) {
    string value;
    try {
      value = med->getStoreValueByIndex(i);
    } catch(MedException &ex) {}
    QModelIndex modelIndex = index(i, ADDRESS_COL_VALUE);
    setData(modelIndex, QString::fromStdString(value));
  }
}

void StoreTreeModel::refresh() {
  this->clearAll();
  for(int i=0;i<med->addresses.size();i++) {
    string address = med->getStoreAddressByIndex(i);
    string value;
    try {
      value = med->getAddressValueByIndex(i);
    } catch(MedException &ex) {}
    string description = med->getStoreDescriptionByIndex(i);
    bool lock = med->getStoreLockByIndex(i);
    QVector<QVariant> data;
    data << description.c_str() << address.c_str() << med->addresses[i]->getScanType().c_str() << value.c_str() << lock;
    TreeItem* childItem = new TreeItem(data, this->root());
    this->appendRow(childItem);
  }
}

void StoreTreeModel::addRow() {
  int lastIndex = med->addresses.size() - 1;
  string address = med->getStoreAddressByIndex(lastIndex);
  string value;
  try {
    value = med->getAddressValueByIndex(lastIndex);
  } catch(MedException &ex) {}
  string description = med->getStoreDescriptionByIndex(lastIndex);
  bool lock = med->getStoreLockByIndex(lastIndex);
  QVector<QVariant> data;
  data << description.c_str() << address.c_str() << med->addresses[lastIndex]->getScanType().c_str() << value.c_str() << lock;
  TreeItem* childItem = new TreeItem(data, this->root());
  this->appendRow(childItem);
}

void StoreTreeModel::sortByAddress() {
  med->sortStoreByAddress();
  refresh();
}

void StoreTreeModel::sortByDescription() {
  med->sortStoreByDescription();
  refresh();
}

void StoreTreeModel::empty() {
  med->addresses.clear();
  TreeModel::empty();
}
