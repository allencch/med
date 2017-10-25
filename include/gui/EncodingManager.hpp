#ifndef ENCODING_MANAGER_HPP
#define ENCODING_MANAGER_HPP

#include <QObject>
#include "med/MedTypes.hpp"

class MainUi; // Forward declaration

class EncodingManager : public QObject {
  Q_OBJECT
public:
  EncodingManager(MainUi* mainUi);
  EncodingType getEncodingType();
  string convertToUtf8(string text);
  string encode(string text);

private slots:
  void onDefaultEncodingTriggered(bool value);
  void onBig5EncodingTriggered(bool value);

private:
  MainUi* mainUi;
  QWidget* mainWindow;
  void setupSubmenu();
  void setupSignals();
  EncodingType encodingType;
};

#endif
