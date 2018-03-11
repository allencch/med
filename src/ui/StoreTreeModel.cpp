// NOTE: This can hardly be refactored with TreeModel. The main reason is the engine Med will do
// different work. Unless Med scan address and store address are abstracted.

#include <QtWidgets>
#include <iostream>
#include <cstdio>

#include "med/MedException.hpp"
#include "ui/Ui.hpp"
#include "ui/EncodingManager.hpp"
#include "ui/TreeItem.hpp"
#include "ui/TreeModel.hpp"
#include "ui/StoreTreeModel.hpp"

using namespace std;

StoreTreeModel::StoreTreeModel(MedUi* mainUi, QObject* parent) : TreeModel(mainUi, parent) {
  QVector<QVariant> rootData;
  rootData << "Description +" << "Address +" << "Type" << "Value" << "Lock";
  rootItem = new TreeItem(rootData);

  this->mainUi = mainUi;
  this->med = mainUi->med;
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
    // TODO: set lock
    // med->setStoreLockByIndex(index.row(), value.toBool());
  }
  else if (index.column() == STORE_COL_DESCRIPTION) {
    // TODO set description
    // med->setStoreDescriptionByIndex(index.row(), value.toString().toStdString());
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

  auto store = med->getStore();
  for (int i = 0; i < rowCount(); i++) {
    string value;
    try {
      value = store->getValue(i);
    } catch(MedException &ex) {}
    QModelIndex modelIndex = index(i, STORE_COL_VALUE);
    setItemData(modelIndex, QString::fromStdString(value));
  }
  emit dataChanged(first, last);
}

void StoreTreeModel::refresh() {
  this->clearAll();
  auto store = med->getStore();
  for (size_t i = 0; i < store->size(); i++) {
    string address = store->getAddressAsString(i);
    string value;
    try {
      value = store->getValue(i);
    } catch(MedException &ex) {
      cerr << "Exception throw in refresh" << endl;
    }

    // TODO: description and lock
    // string description = med->getStoreDescriptionByIndex(i);
    // bool lock = med->getStoreLockByIndex(i);
    string description = "nothing";
    bool lock = false;

    QVector<QVariant> data;
    data << description.c_str() <<
      address.c_str() <<
      store->getScanType(i).c_str() <<
      value.c_str() <<
      lock;

    TreeItem* childItem = new TreeItem(data, this->root());
    this->appendRow(childItem);
  }
}

void StoreTreeModel::addRow() {
  auto store = med->getStore();
  int lastIndex = store->getLastIndex();
  string address = store->getAddressAsString(lastIndex);
  string value;
  try {
    value = store->getValue(lastIndex);
  } catch(MedException &ex) {
    cerr << "Add row no value" << endl;
  }
  // TODO: get description and lock
  // string description = med->getStoreDescriptionByIndex(lastIndex);
  // bool lock = med->getStoreLockByIndex(lastIndex);

  string description = "nothing more";
  bool lock = false;
  QVector<QVariant> data;
  data << description.c_str() <<
    address.c_str() <<
    store->getScanType(lastIndex).c_str() <<
    value.c_str() <<
    lock;
  TreeItem* childItem = new TreeItem(data, this->root());
  this->appendRow(childItem);
}

void StoreTreeModel::sortByAddress() {
  med->getStore()->sortByAddress();
  refresh();
}

void StoreTreeModel::sortByDescription() {
  // med->sortStoreByDescription(); // TODO: sort by description
  refresh();
}

void StoreTreeModel::empty() {
  med->getStore()->clear();
  TreeModel::empty();
}

void StoreTreeModel::setValue(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    string scanType = med->getStore()->getScanType(row);
    string newValue = encodeString(value.toString().toStdString(), scanType);

    med->getStore()->setValue(row, newValue, scanType);
  } catch(MedException &e) {
    cerr << "editStoreValue: " << e.what() << endl;
  }
}

void StoreTreeModel::setType(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    med->getStore()->setScanType(row, value.toString().toStdString());
    string valueByAddress = med->getStore()->getValue(row, value.toString().toStdString());
    QVariant valueToSet = getUtfString(row, value.toString().toStdString());

    setItemData(this->index(index.row(), STORE_COL_VALUE), valueToSet); //Update the target value
  } catch(MedException &e) {
    cerr << "editStoreType: " << e.what() << endl;
  }
}


void StoreTreeModel::setAddress(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    med->getStore()->setAddress(row, value.toString().toStdString());
    string value2 = med->getStore()->getValue(row);
    QVariant valueToSet = QString::fromStdString(value2);

    setItemData(this->index(index.row(), STORE_COL_VALUE), valueToSet); //Update the target value
  } catch(MedException &e) {
    cerr << "editStoreAddress: " << e.what() << endl;
  }
}


bool StoreTreeModel::setItemData(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  TreeItem *item = getItem(index);
  string scanType = med->getStore()->getScanType(row);

  QVariant newValue = value;

  if (index.column() == STORE_COL_VALUE && scanType == SCAN_TYPE_STRING) {
    newValue = getUtfString(row, scanType);
  }
  return item->setData(index.column(), newValue);
}

QVariant StoreTreeModel::getUtfString(int row, string scanType) {
  string valueByAddress = med->getStore()->getValue(row, scanType);
  string utfString = mainUi->encodingManager->convertToUtf8(valueByAddress);
  return QString::fromStdString(utfString);
}
