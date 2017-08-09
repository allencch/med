#ifndef MEM_EDITOR_HPP
#define MEM_EDITOR_HPP

#include <QWidget>
#include "med/med.hpp"

class MemEditor : public QWidget {
  Q_OBJECT

public:
  MemEditor(QWidget* parent, Med* med);

private:
  Med* med;
  QWidget* parent;
  QWidget* mainChild;
};

#endif
