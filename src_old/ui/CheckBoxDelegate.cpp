#include <QtWidgets>
#include <QtDebug>
#include <cstdio>
#include <iostream>
#include "ui/CheckBoxDelegate.hpp"

using namespace std;

CheckBoxDelegate::CheckBoxDelegate(QWidget* parent) : QStyledItemDelegate(parent) {
  editor = NULL;
}

QWidget* CheckBoxDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  QCheckBox* editor = new QCheckBox(parent);

  QObject::connect(editor, SIGNAL(stateChanged(int)), this, SLOT(setData(int)));

  return editor;
}


void CheckBoxDelegate::setEditorData(QWidget* editor,
                                     const QModelIndex &index) const {
  bool value = !index.model()->data(index, Qt::DisplayRole).toBool();
  QCheckBox* checkBox = static_cast<QCheckBox*>(editor);
  checkBox->setCheckState(value ? Qt::Checked : Qt::Unchecked);
}

void CheckBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                    const QModelIndex &index) const {
  QCheckBox* checkBox = static_cast<QCheckBox*>(editor);
  bool value = checkBox->checkState() == Qt::Checked ? true : false;
  model->setData(index, value);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}

void CheckBoxDelegate::setData(int value) {
  emit commitData(editor);
}

void CheckBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QStyleOptionButton checkBoxOption;
  checkBoxOption.rect = option.rect;
  checkBoxOption.state = index.model()->data(index).toBool() ? QStyle::State_On : QStyle::State_Off;
  QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}


QSize CheckBoxDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QCheckBox checkbox;
  return checkbox.sizeHint(); //Fix the height issue
  //return QStyledItemDelegate::sizeHint(option, index);
}
