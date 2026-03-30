#ifndef TREEITEM_H
#define TREEITEM_H

#include <QApplication>
#include <QVector>
#include <QVariant>

class TreeItem {
public:
  explicit TreeItem(const QVector<QVariant> &data, TreeItem* parent = 0);
  ~TreeItem();

  void appendChild(TreeItem* item);

  TreeItem* child(int row);

  int childCount() const;

  int row() const;
  int columnCount() const;
  TreeItem* parent();
  QVariant data(int column) const;

  bool insertChildren(int position, int count, int columns);

  bool insertColumns(int position, int columns);

  bool removeChildren(int position, int count);
  bool removeColumns(int position, int columns);

  int childNumber() const;

  bool setData(int column, const QVariant &value);
private:
  TreeItem* parentItem;
  QVector<QVariant> itemData;
  QList<TreeItem*> childItems;
};

#endif
