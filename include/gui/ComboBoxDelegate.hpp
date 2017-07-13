#ifndef COMBOBOX_DELEGATE_H
#define COMBOBOX_DELEGATE_H

/**
 * Issue:
 * There is a remaining issue using ComboBoxDelegate. It will only
 * appear when editing, not by default like the QTreeWidget.
 */

//Based on http://www.qtcentre.org/threads/43148-QComboBox-in-QTreeView
// and http://doc.qt.io/qt-5/qtwidgets-itemviews-spinboxdelegate-example.html
#include <QStyledItemDelegate>
#include <QComboBox>

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

  mutable QComboBox* editor; //Not using yet

  void paint(QPainter* painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const Q_DECL_OVERRIDE;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const Q_DECL_OVERRIDE;

private slots:
  void setData(int value);
};

#endif
