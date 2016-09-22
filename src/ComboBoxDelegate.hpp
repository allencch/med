#ifndef COMBOBOX_DELEGATE_H
#define COMBOBOX_DELEGATE_H

//Based on http://www.qtcentre.org/threads/43148-QComboBox-in-QTreeView
#include <QStyledItemDelegate>

class ComboBoxDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  ComboBoxDelegate(QWidget* parent = 0) : QStyledItemDelegate(parent) {}

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setEditorData(QWidget* editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex &index) const Q_DECL_OVERRIDE;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
