#include <QtWidgets>
#include <cstdio>
#include "ComboBoxDelegate.hpp"

QWidget* ComboBoxDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  QComboBox* editor = new QComboBox(parent);

  //Based on http://stackoverflow.com/questions/23452559/make-the-delegate-qtcombobox-able-to-detect-clicks
  QObject::connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(setData(int)));

  editor->addItems(QStringList() <<
                    "int8" <<
                    "int16" <<
                    "int32" <<
                    "float32" <<
                    "float64");
  return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor,
                                     const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::DisplayRole).toString();
  QComboBox* comboBox = static_cast<QComboBox*>(editor);
  int selectedIndex = comboBox->findText(value);
  comboBox->setCurrentIndex(selectedIndex);
  comboBox->showPopup();
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

void ComboBoxDelegate::setData(int value) {
  emit commitData(editor);
}


void ComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  //From: http://programmingexamples.net/wiki/Qt/Delegates/ComboBoxDelegate
  // and https://forum.qt.io/topic/18175/qcombobox-in-qtreeview/2
  QStyleOptionComboBox comboBoxOption;
  comboBoxOption.rect = option.rect;
  comboBoxOption.state = option.state;
  comboBoxOption.frame = true;
  comboBoxOption.currentText = index.model()->data(index).toString();

  QApplication::style()->drawComplexControl(QStyle::CC_ComboBox, &comboBoxOption, painter);
  QApplication::style()->drawControl(QStyle::CE_ComboBoxLabel, &comboBoxOption, painter);
}


QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QComboBox combo;
  return combo.sizeHint(); //Fix the height issue
  //return QStyledItemDelegate::sizeHint(option, index);
}
//*/
