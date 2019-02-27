#include "messenger.h"

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpSocket>


Messenger::Messenger() : QObject() {
    auto serverName = QHostInfo::localHostName();
    socket_ = new QTcpSocket( this );
    connect( socket_, SIGNAL( connected() ), this, SLOT( initializeConnection() ) );
    connect( socket_, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( disconnect() ) );
    connect( socket_, SIGNAL( disconnected() ), this, SLOT( disconnect() ) );
    connect( socket_, SIGNAL( readyRead() ), this, SLOT( receiveText() ) );
    socket_->connectToHost( serverName, 12345 );
}


Messenger::~Messenger() {
}


void Messenger::initializeConnection() {
    // nickname and chat server version hardcoded for now
    send( "/1.2/UnspecifiedNickname" );
}


void Messenger::disconnect() {
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