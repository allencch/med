#ifndef CHECK_BOX_DELEGATE_HPP
#define CHECK_BOX_DELEGATE_HPP

#include <QStyledItemDelegate>
#include <QCheckBox>

class CheckBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit CheckBoxDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    
    // We don't really need a full editor widget if we just want to toggle on click
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif
