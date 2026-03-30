#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "ui/TreeItem.hpp"
#include "mem/MemEd.hpp"

/**
 * Trying to use the Med instead of TreeItem. But looks like using Med directly instead of TreeItem looks improbable.
 * Because the TreeItem will represent the cell in the TreeView. So, Med cannot directly represented
 * as the cell, unless through TreeItem, because of the QVariant(). Therefore,
 * I put Med in the model, so that, instead of writing the interaction through the Main(), it will be
 * easier writing the interactions of Model and Med within the Model itself.
 */

class MedUi;

class TreeModel : public QAbstractItemModel {
  Q_OBJECT
public:
  TreeModel(MedUi* mainUi, QObject* parent = 0);
  ~TreeModel();

  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

  bool setHeaderData(int section, Qt::Orientation orientation,
                     const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

  bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

  bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

  bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

  bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;


  QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

  int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

  int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

  //My implementation
  void appendRow(TreeItem* treeItem);
  void clearAll();

  void addScan(string scanType);

  void refreshValues();
  void empty(); //including the med data

  TreeItem* root();
  MedUi* mainUi;
  MemEd* med;

protected:
  void setupModelData(const QStringList &lines, TreeItem* parent);

  TreeItem* getItem(const QModelIndex &index) const;
  TreeItem* rootItem;

  void setValue(const QModelIndex &index, const QVariant &value);
  void setType(const QModelIndex &index, const QVariant &value);

  bool setItemData(const QModelIndex &index, const QVariant &value);
  QVariant getUtfString(int row, string scanType);
  string encodeString(const string& str, const string& scanType);
  string convertToUtf8(const string& str, const string& scanType);
};

#endif // TREEMODEL_H
