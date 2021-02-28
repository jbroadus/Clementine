#ifndef INTERFACE_COMPONENT_H
#define INTERFACE_COMPONENT_H

#include <QList>
#include <QObject>

namespace IClementine {
  class ComponentInterface : public QObject {
   public:
    ComponentInterface(QObject* parent = nullptr) : QObject(parent) {}

    virtual const QString GetName() = 0;
  };

  typedef QList<ComponentInterface*> ComponentInterfaceList;
}

#endif  // INTERFACE_COMPONENT_H
