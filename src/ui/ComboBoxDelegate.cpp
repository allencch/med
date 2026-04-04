#include "ui/ComboBoxDelegate.hpp"
#include "med/MedCommon.hpp"

ComboBoxDelegate::ComboBoxDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    QComboBox* editor = new QComboBox(parent);
    editor->addItem(QString::fromStdString(ScanTypeString::Int8));
    editor->addItem(QString::fromStdString(ScanTypeString::UInt8));
    editor->addItem(QString::fromStdString(ScanTypeString::Int16));
    editor->addItem(QString::fromStdString(ScanTypeString::UInt16));
    editor->addItem(QString::fromStdString(ScanTypeString::Int32));
    editor->addItem(QString::fromStdString(ScanTypeString::UInt32));
    editor->addItem(QString::fromStdString(ScanTypeString::Int64));
    editor->addItem(QString::fromStdString(ScanTypeString::UInt64));
    editor->addItem(QString::fromStdString(ScanTypeString::Float32));
    editor->addItem(QString::fromStdString(ScanTypeString::Float64));
    editor->addItem(QString::fromStdString(ScanTypeString::Ptr32));
    editor->addItem(QString::fromStdString(ScanTypeString::Ptr64));
    editor->addItem(QString::fromStdString(ScanTypeString::String));
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentText(value);
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const {
    editor->setGeometry(option.rect);
}
