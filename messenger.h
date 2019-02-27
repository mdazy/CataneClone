#pragma once

#include <QtCore/QObject>

class QTcpSocket;

class Messenger : public QObject {
    Q_OBJECT;
public:
    Messenger();
    virtual ~Messenger();

private slots:
    void initializeConnection();
    void receiveText();
    void disconnect();

private:
    QTcpSocket* socket_;
};