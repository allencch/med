#include <QtWidgets>
#include <iostream>
#include <cstdio>
#include "gui/med-qt.hpp"
#include "gui/TreeItem.hpp"
#include "gui/TreeModel.hpp"
#include "gui/StoreTreeModel.hpp"

using namespace std;

StoreTreeModel::StoreTreeModel(MainUi* mainUi, QObject* parent) : TreeModel(mainUi, parent) {
  QVector<QVariant> rootData;
  rootData << "Description +" << "Address +" << "Type" << "Value" << "Lock";
  rootItem = new TreeItem(rootData);

  this->mainUi = mainUi;
  this->med = &(mainUi->med);
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
  if (role == Qt::CheckStateRole && index.column() == STORE_COL_LOCK) {
    return item->data(index.column()).toBool() ? Qt::Checked : Qt::Unchecked;
  }

  return item->data(index.column());
}

bool StoreTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role != Qt::EditRole)
    return false;

  if(index.column() == STORE_COL_VALUE) {
    setValue(index, value);
  }
  else if (index.column() == STORE_COL_TYPE) {
    setType(index, value);
  }
  else if (index.column() == STORE_COL_ADDRESS) {
    setAddress(index, value);
  }
  else if (index.column() == STORE_COL_LOCK) {
    med->setStoreLockByIndex(index.row(), value.toBool());
  }
  else if (index.column() == STORE_COL_DESCRIPTION) {
    med->setStoreDescriptionByIndex(index.row(), value.toString().toStdString());
  }

  bool result = setItemData(index, value); //Update the cell

  if (result) {
    emit dataChanged(index, index);
  }

  return result;
}

Qt::ItemFlags StoreTreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return 0;
  Qt::ItemFlags flags = Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  if (index.column() == STORE_COL_LOCK) {
    flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

void StoreTreeModel::refreshValues() {
  if (rowCount() == 0)
    return;
  QModelIndex first = index(0, STORE_COL_VALUE);
  QModelIndex last = index(rowCount() - 1, STORE_COL_VALUE);

  for(int i=0; i < rowCount(); i++) {
    string value;
    try {
      value = med->getStoreValueByIndex(i);
    } catch(MedException &ex) {}
    QModelIndex modelIndex = index(i, STORE_COL_VALUE);
    setItemData(modelIndex, QString::fromStdString(value));
  }
  emit dataChanged(first, last);
}

void StoreTreeModel::refresh() {
  this->clearAll();
  for(int i = 0; i < (int)med->addresses.size(); i++) {
    string address = med->getStoreAddressByIndex(i);
    string value;
    try {
      value = med->getStoreValueByIndex(i);
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
    value = med->getStoreValueByIndex(lastIndex);
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
  med->clearStore();
  TreeModel::empty();
}

void StoreTreeModel::setValue(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    med->addresses[row]->setLockedValue(value.toString().toStdString());
    med->setValueByAddress(med->addresses[row]->address,
                           value.toString().toStdString(),
                           med->addresses[row]->getScanType());
  } catch(MedException &e) {
    cerr << "editStoreValue: " << e.what() << endl;
  }
}

void StoreTreeModel::setType(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    med->addresses[row]->setScanType(value.toString().toStdString());
    string valueByAddress = med->getValueByAddress(med->addresses[row]->address,
                                                   value.toString().toStdString());
    QVariant valueToSet = getUtfString(row, value.toString().toStdString());

    setItemData(this->index(index.row(), STORE_COL_VALUE), valueToSet); //Update the target value
  } catch(MedException &e) {
    cerr << "editStoreType: " << e.what() << endl;
  }
}


void StoreTreeModel::setAddress(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    string value2 = med->setStoreAddressByIndex(row, value.toString().toStdString());
    QVariant valueToSet = QString::fromStdString(value2);

    setItemData(this->index(index.row(), STORE_COL_VALUE), valueToSet); //Update the target value
  } catch(MedException &e) {
    cerr << "editStoreAddress: " << e.what() << endl;
  }
}


bool StoreTreeModel::setItemData(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  TreeItem *item = getItem(index);
  string scanType = med->addresses[row]->getScanType();

  QVariant newValue = value;

  if (index.column() == STORE_COL_VALUE && scanType == SCAN_TYPE_STRING) {
    newValue = getUtfString(row, scanType);
  }
  return item->setData(index.column(), newValue);
}
