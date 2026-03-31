#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include "ui/CheckBoxDelegate.hpp"

CheckBoxDelegate::CheckBoxDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void CheckBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    bool checked = index.model()->data(index, Qt::EditRole).toBool();

    QStyleOptionButton checkBoxOption;
    QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxOption);
    checkBoxOption.rect = option.rect;
    checkBoxOption.rect.setLeft(option.rect.left() + (option.rect.width() - checkBoxRect.width()) / 2);
    checkBoxOption.state = QStyle::State_Enabled | (checked ? QStyle::State_On : QStyle::State_Off);

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

bool CheckBoxDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem&, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        bool checked = index.model()->data(index, Qt::EditRole).toBool();
        model->setData(index, !checked, Qt::EditRole);
        return true;
    }
    return false;
}

QWidget* CheckBoxDelegate::createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const {
    return nullptr; // No editor needed
}
