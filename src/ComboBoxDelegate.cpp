#include <QtWidgets>
#include "ComboBoxDelegate.hpp"

QWidget* ComboBoxDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  QComboBox* editor = new QComboBox(parent);
  QStringList list ;
  list << "a" << "b" << "c" << "d";
  editor->addItems(list);
  return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor,
                                     const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::DisplayRole).toString();
  QComboBox* comboBox = static_cast<QComboBox*>(editor);
  comboBox->addItem(value);
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                    const QModelIndex &index) const {
  QComboBox* comboBox = static_cast<QComboBox*>(editor);
  QString value = comboBox->currentText();
  model->setData(index, value);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}
