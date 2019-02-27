#include "messenger.h"

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpSocket>


Messenger::Messenger() : QObject() {
    auto serverName = QHostInfo::localHostName();
    socket_ = new QTcpSocket( this );
    connect( socket_, &QTcpSocket::connected, this, &Messenger::initializeConnection );
    connect( socket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Messenger::disconnectFromServer );
    connect( socket_, &QTcpSocket::disconnected, this, &Messenger::disconnectFromServer );
    connect( socket_, &QTcpSocket::readyRead, this, &Messenger::receiveText );
    socket_->connectToHost( serverName, 12345 );
}


Messenger::~Messenger() {
}


void Messenger::initializeConnection() {
    // nickname and chat server version hardcoded for now
    send( "/1.2/UnspecifiedNickname" );
}


void Messenger::disconnectFromServer() {
    // well, duh
}


void Messenger::receiveText() {
    auto text = QString::fromLocal8Bit( socket_->readAll() );
    if( text.startsWith( "/gameCmd" ) ) {
        emit gameCommand( text.right( 9 ) );
    } else if( text.startsWith( "/gameInfo" ) ) {
        emit chatMessage( text.right( 10 ) );
    } else {
        emit chatMessage( text );
    }
}


void Messenger::sendChatMessage( const QString& text ) const {
    send( text );
}


void Messenger::send( const QString& text ) const {
    socket_->write( text.toLocal8Bit() );
    socket_->flush();
}