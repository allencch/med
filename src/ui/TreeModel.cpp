#include <QtWidgets>
#include <iostream>
#include <cstdio>

#include "med/MedException.hpp"
#include "ui/Ui.hpp"
#include "ui/EncodingManager.hpp"
#include "ui/TreeItem.hpp"
#include "ui/TreeModel.hpp"

using namespace std;

TreeModel::TreeModel(MedUi* mainUi, QObject* parent) : QAbstractItemModel(parent) {
  QVector<QVariant> rootData;
  rootData << "Address" << "Type" << "Value";
  rootItem = new TreeItem(rootData);

  this->mainUi = mainUi;
  this->med = mainUi->med;
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

  if (index.column() == SCAN_COL_VALUE) {
    setValue(index, value);
  }
  else if (index.column() == SCAN_COL_TYPE) {
    setType(index, value);
  }

  bool result = setItemData(index, value); //Update the cell

  if (result) {
    emit dataChanged(index, index);
  }

  return result;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::NoItemFlags;

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

  beginRemoveRows(parent, position, position + rows - 1);
  bool success = parentItem->removeChildren(position, rows);
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


// My Implementation

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
  auto scans = med->getScans();
  for(size_t i = 0; i < scans.size(); i++) {
    string address = scans.getAddressAsString(i);
    string value = scans.getValue(i, scanType);

    string newValue = convertToUtf8(value, scanType);

    QVector<QVariant> data;
    data << address.c_str() << scanType.c_str() << newValue.c_str();
    TreeItem* childItem = new TreeItem(data, this->root());
    this->appendRow(childItem);
  }
}

void TreeModel::refreshValues() {
  if (rowCount() == 0) {
    return;
  }
  QModelIndex first = index(0, SCAN_COL_VALUE);
  QModelIndex last = index(rowCount() - 1, SCAN_COL_VALUE);
  auto scans = med->getScans();
  for (int i = 0; i < rowCount(); i++) {
    string value = scans.getValue(i);
    QModelIndex modelIndex = index(i, SCAN_COL_VALUE);
    setItemData(modelIndex, QString::fromStdString(value));
  }
  emit dataChanged(first, last);
}

void TreeModel::empty() {
  med->clearScans();
  clearAll();
}

void TreeModel::setValue(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    string scanType = med->getScans().getScanType(row);
    string newValue = encodeString(value.toString().toStdString(), scanType);
    med->getScans().setValue(row, newValue, scanType);
  } catch(MedException &e) {
    cerr << "editScanValue: " << e.what() << endl;
  }
}

void TreeModel::setType(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  try {
    med->getScans().setScanType(row, value.toString().toStdString());

    QVariant valueToSet = getUtfString(row, value.toString().toStdString());

    setItemData(this->index(index.row(), SCAN_COL_VALUE), valueToSet); // Update the target value
  } catch(MedException &e) {
    cerr << "editScanType: " << e.what() << endl;
  }
}

bool TreeModel::setItemData(const QModelIndex &index, const QVariant &value) {
  int row = index.row();
  TreeItem *item = getItem(index);
  try {
    string scanType = med->getScans().getScanType(row);

    QVariant newValue = value;

    if (index.column() == SCAN_COL_VALUE && scanType == SCAN_TYPE_STRING) {
      newValue = getUtfString(row, scanType);
    }
    return item->setData(index.column(), newValue);
  } catch (const MedException& e) {
    cout << "Exception tree model set data: " << e.what() << endl;
    return false;
  }
}

QVariant TreeModel::getUtfString(int row, string scanType) {
  string valueByAddress = med->getScans().getValue(row, scanType);
  string utfString = mainUi->encodingManager->convertToUtf8(valueByAddress);
  return QString::fromStdString(utfString);
}

string TreeModel::encodeString(const string& str, const string& scanType) {
  if (scanType == SCAN_TYPE_STRING) {
    return mainUi->encodingManager->encode(str);
  }
  return str;
}

string TreeModel::convertToUtf8(const string& str, const string& scanType) {
  if (scanType == SCAN_TYPE_STRING) {
    return mainUi->encodingManager->convertToUtf8(str);
  }
  return str;
}
