#include <QApplication>
#include <QtUiTools>
#include "ui/TreeItem.hpp"

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem* parent) {
  this->parentItem = parent;
  this->itemData = data;
}
TreeItem::~TreeItem() {
  qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem* item) {
  childItems.append(item);
}

TreeItem* TreeItem::child(int row) {
  return childItems.value(row);
}

int TreeItem::childCount() const {
  return childItems.count();
}

int TreeItem::row() const {
  if(parentItem)
    return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
  return 0;
}
int TreeItem::columnCount() const {
  return itemData.count();
}
TreeItem* TreeItem::parent() {
  return parentItem;
}
QVariant TreeItem::data(int column) const {
  return itemData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns) {
  if(position < 0 || position > childItems.size())
    return false;

  for(int row=0 ; row < count ; row++) {
    QVector<QVariant> data(columns);
    TreeItem* item = new TreeItem(data, this);
    childItems.insert(position, item);
  }
  return true;
}

bool TreeItem::insertColumns(int position, int columns) {
  if(position < 0 || position > itemData.size())
    return false;
  for(int column = 0; column < columns; column++) {
    itemData.insert(position, QVariant());
  }
  foreach(TreeItem* child, childItems) //Qt keyword
    child->insertColumns(position, columns);
  return true;
}

bool TreeItem::removeChildren(int position, int count) {
  if(position < 0 || position + count > childItems.size())
    return false;
  for(int row = 0; row < count; row++) {
    delete childItems.takeAt(position);
  }
  return true;
}
bool TreeItem::removeColumns(int position, int columns) {
  if (position < 0 || position + columns > itemData.size())
    return false;

  for (int column = 0; column < columns; ++column)
    itemData.remove(position);

  foreach (TreeItem *child, childItems)
    child->removeColumns(position, columns);

  return true;
}

int TreeItem::childNumber() const { //Same as row()
  if(parentItem)
    return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
  return 0;
}

bool TreeItem::setData(int column, const QVariant &value) {
  if(column <0 || column >= itemData.size()) {
    return false;
  }
  itemData[column] = value;
  return true;
}
