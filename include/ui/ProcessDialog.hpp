#ifndef PROCESS_DIALOG_HPP
#define PROCESS_DIALOG_HPP

#include <QDialog>
#include <QTreeWidget>
#include <vector>
#include "med/Process.hpp"

class ProcessDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProcessDialog(QWidget* parent = nullptr);
    void setProcessList(const std::vector<Process>& processes);

protected:
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void processSelected(pid_t pid, const QString& name);

private slots:
    void onDoubleClicked(QTreeWidgetItem* item, int column);

private:
    QTreeWidget* treeWidget_;
};

#endif
