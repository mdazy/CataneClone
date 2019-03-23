#include "messenger.h"

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpSocket>

#include <iostream>
using namespace std;

const QString CMD = "/gameCmd/";
const QString INFO = "/gameInfo/";


Messenger::Messenger( int port, QObject* parent ) : QObject( parent ) {
    auto serverName = QHostInfo::localHostName();
    socket_ = new QTcpSocket( this );
    connect( socket_, &QTcpSocket::connected, this, &Messenger::initializeConnection );
    connect( socket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Messenger::disconnectFromServer );
    connect( socket_, &QTcpSocket::disconnected, this, &Messenger::disconnectFromServer );
    connect( socket_, &QTcpSocket::readyRead, this, &Messenger::receiveText );
    socket_->connectToHost( serverName, port );
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
    // append received text to pending buffer
    buffer_ += QString::fromLocal8Bit( socket_->readAll() );

    // find length of next sequence
    int slashPos = buffer_.indexOf( "/" );
    int len = buffer_.left( slashPos ).toInt();
    // if buffer contains the expected length, process
    while( !buffer_.isEmpty() && len > 0 && buffer_.length() >= len + slashPos + 1 ) {
        // skip the sequence length and get the actual text
        auto text = buffer_.mid( slashPos + 1, len );
        // process
        if( text.startsWith( CMD ) ) {
            emit gameCommand( text.mid( CMD.length() ) );
        } else if( text.startsWith( INFO ) ) {
            emit chatMessage( text.mid( INFO.length() ) );
        } else {
            emit chatMessage( text );
        }
        // discard the processed sequence length and text
        buffer_ = buffer_.mid( slashPos + 1 + len );
        slashPos = buffer_.indexOf( "/" );
        len = buffer_.left( slashPos ).toInt();
    }
}


void Messenger::sendChatMessage( const QString& text ) const {
    send( text );
}

void Messenger::sendGameCommand( const QString& cmd ) const {
    send( CMD + cmd );
}

void Messenger::send( const QString& text ) const {
    socket_->write( ( QString( "%1/").arg( text.length() ) + text) .toLocal8Bit() );
    socket_->flush();
}