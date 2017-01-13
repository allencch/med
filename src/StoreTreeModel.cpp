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
  rootData << "Description" << "Address" << "Type" << "Value" << "Lock";
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

  //Update the med
  //Update, split this out
  int row = index.row();
  if(index.column() == ADDRESS_COL_VALUE) {
    try {
      med->setValueByAddress(med->addresses[row].address,
                             value.toString().toStdString(),
                             med->addresses[row].getScanType());
    } catch(string e) {
      cerr << "editStoreValue: "<<e<<endl;
    }
  }
  else if (index.column() == ADDRESS_COL_TYPE) {
    try {
      med->addresses[row].setScanType(value.toString().toStdString());
      string value2 = med->getValueByAddress(med->addresses[row].address,
                                             value.toString().toStdString());
      QVariant valueToSet = QString::fromStdString(value2);

      TreeItem *item = getItem(index);
      item->setData(ADDRESS_COL_VALUE, valueToSet); //Update the target value
    } catch(string e) {
      cerr << "editStoreType: " << e << endl;
    }
  }
  else if (index.column() == ADDRESS_COL_ADDRESS) {
    try {
      med->addresses[row].address = hexToInt(value.toString().toStdString());
      string value2 = med->getValueByAddress(med->addresses[row].address,
                                             med->addresses[row].getScanType());
      QVariant valueToSet = QString::fromStdString(value2);

      TreeItem *item = getItem(index);
      item->setData(ADDRESS_COL_VALUE, valueToSet); //Update the target value
     } catch(string e) {
      cerr << "editStoreAddress: " << e << endl;
    }
  }
  else if (index.column() == ADDRESS_COL_LOCK) {
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

void StoreTreeModel::addScan() {
  this->clearAll();
  for(int i=0;i<med->addresses.size();i++) {
    char address[32];
    sprintf(address, "%p", (void*)(med->addresses[i].address));
    string value = med->getAddressValueByIndex(i);
    QVector<QVariant> data;
    data << "Your description" << address << med->addresses[i].getScanType().c_str() << value.c_str() << false;
    TreeItem* childItem = new TreeItem(data, this->root());
    this->appendRow(childItem);
  }
}
