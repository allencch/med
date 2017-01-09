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
