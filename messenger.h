#pragma once

#include <QtCore/QObject>

class QTcpSocket;

class Messenger : public QObject {
    Q_OBJECT;
public:
    Messenger( int port, QObject* parent = Q_NULLPTR );
    virtual ~Messenger();

public:
    void sendChatMessage( const QString& text ) const;

private slots:
    void initializeConnection();
    void receiveText();
    void disconnectFromServer();

signals:
    void gameCommand( const QString& cmd );
    void chatMessage( const QString& text );

private:
    void send( const QString& message ) const;

private:
    QTcpSocket* socket_;
};