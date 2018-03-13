#ifndef CHECKBOX_DELEGATE_H
#define CHECKBOX_DELEGATE_H

#include <QStyledItemDelegate>
#include <QCheckBox>

class CheckBoxDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  CheckBoxDelegate(QWidget* parent = 0) : QStyledItemDelegate(parent) {}


  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setEditorData(QWidget* editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex &index) const Q_DECL_OVERRIDE;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

  mutable QCheckBox* editor; //Not using yet

  void paint(QPainter* painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const Q_DECL_OVERRIDE;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const Q_DECL_OVERRIDE;
private slots:
  void setData(int value);
};
#endif
