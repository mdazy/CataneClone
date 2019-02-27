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
}


void Messenger::disconnect() {
}


void Messenger::receiveText() {
}