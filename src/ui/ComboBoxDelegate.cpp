#include <QApplication>
#include "ui/ComboBoxDelegate.hpp"
#include "med/MedCommon.hpp"

ComboBoxDelegate::ComboBoxDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    QComboBox* editor = new QComboBox(parent);
    editor->addItem(QString::fromStdString(ScanTypeString::Int8));
    editor->addItem(QString::fromStdString(ScanTypeString::Int16));
    editor->addItem(QString::fromStdString(ScanTypeString::Int32));
    editor->addItem(QString::fromStdString(ScanTypeString::Int64));
    editor->addItem(QString::fromStdString(ScanTypeString::Float32));
    editor->addItem(QString::fromStdString(ScanTypeString::Float64));
    editor->addItem(QString::fromStdString(ScanTypeString::Ptr32));
    editor->addItem(QString::fromStdString(ScanTypeString::Ptr64));
    editor->addItem(QString::fromStdString(ScanTypeString::String));

    connect(editor, &QComboBox::activated, this, [this, editor]() {
        emit const_cast<ComboBoxDelegate*>(this)->commitData(editor);
        emit const_cast<ComboBoxDelegate*>(this)->closeEditor(editor);
    });
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentText(value);
    comboBox->showPopup();
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const {
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionComboBox comboBoxOption;
    comboBoxOption.rect = option.rect;
    comboBoxOption.state = option.state;
    comboBoxOption.frame = true;
    comboBoxOption.currentText = index.data(Qt::EditRole).toString();

    // Draw the combo box frame and arrow
    QApplication::style()->drawComplexControl(QStyle::CC_ComboBox, &comboBoxOption, painter);
    // Draw the actual text
    QApplication::style()->drawControl(QStyle::CE_ComboBoxLabel, &comboBoxOption, painter);
}

QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
    QComboBox combo;
    return combo.sizeHint(); //Fix the height issue
    //return QStyledItemDelegate::sizeHint(option, index);
}
