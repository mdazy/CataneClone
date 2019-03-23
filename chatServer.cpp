#include "chatServer.h"

#include <QtCore/QMap>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpSocket>

#include <iostream>
using namespace std;


static const QString version = "1.2";


/*!
 * Convenience for displaying a socket address in standard streams.
 */
ostream& operator<<( ostream& out, const QAbstractSocket* s ) {
    out << s->peerAddress().toString().toStdString() << ":" << s->peerPort();
    return out;
}


/*!
 * Starts the server on the given port.
 */
ChatServer::ChatServer( int port, QObject* parent ) : QTcpServer( parent ) {
    // pick a single IPv4 address for the local host
    auto hostInfo = QHostInfo::fromName( QHostInfo::localHostName() );
    QHostAddress myAddress;
    for( const auto& a : hostInfo.addresses() ) {
        if( a.protocol() != QAbstractSocket::IPv4Protocol ) {
            continue;
        }
        if( ( a.toIPv4Address() & 0xff ) == 1 ) {
            continue;
        }
        myAddress = a;
        break;
    }
    if( !listen( myAddress, port ) ) {
        // TODO: how to diagnose failure
        cerr << "NOT LISTENING" << endl;
    } else {
        connect( this, &ChatServer::newConnection, this, &ChatServer::inspectConnection );
    }
}


/*!
 * Registers a new client with a default empty nickname.
 */
void ChatServer::inspectConnection() {
    while( hasPendingConnections() ) {
        auto socket = nextPendingConnection();
        connect( socket, &QTcpSocket::readyRead, this, &ChatServer::dispatch );
        connect( socket, &QTcpSocket::disconnected, this, &ChatServer::disconnectClient );
    }
}


/*!
 * Sends a message to all clients except \a from if specified.
 */
void ChatServer::sendToClients( const QString& msg, const QTcpSocket* from ) const {
    for( auto s = clientSockets_.keyBegin(); s != clientSockets_.keyEnd(); ++s ) {
        if( *s == from ) {
            continue;
        }
        ( *s )->write( ( QString( "%1/").arg( msg.length() ) + msg) .toLocal8Bit() );
        ( *s )->flush();
    }
}


/*!
 * Processes the message received from the sender.
 *
 * If it is the first message from that sender it is stored as the nickname for that client
 * and a connection notification is sent.
 *
 * If it does not start with the expected nick, the new message is stored as the new nick
 * and a renaming notification is sent.
 *
 * Otherwise the message is sent as-is.
 */
void ChatServer::dispatch() {
    auto from = static_cast<QTcpSocket*>( sender() );
    buffer_ += QString::fromLocal8Bit( from->readAll() );

    int slashPos = buffer_.indexOf( "/" );
    int len = buffer_.left( slashPos ).toInt();
    // if buffer contains the expected length, process
    while( !buffer_.isEmpty() && len > 0 && buffer_.length() >= len + slashPos + 1 ) {
        // skip the sequence length and get the actual text
        auto text = buffer_.mid( slashPos + 1, len );

        if( clientSockets_.constFind( from ) == clientSockets_.constEnd() ) {
            QStringList initText = text.split( "/", QString::SkipEmptyParts );
            if( initText.size() != 2 ) {
                reject( from, "Invalid initialization data." );
                return;
            } else if( initText[ 0 ] != version ) {
                reject( from, "Your client is for an obsolete version, upgrade it." );
            } else {
                clientSockets_.insert( from, initText[ 1 ] );
                sendToClients( "/<b><font color=\"#442222\">" + initText[ 1 ] + " connected.</font></b>", from );
            }
        } else {
                if( !text.startsWith( clientSockets_[ from ] + ": " ) && !text.startsWith( "/") ) {
                // new nick, update and notify
                QString oldNick = clientSockets_[ from ];
                clientSockets_[ from ] = text;
                text = "/<b><font color=\"#442222\">" + oldNick + " has changed his nick to " + text + ".</font></b>";
            } else {
                // regular text
            }
            sendToClients( text, from );
        }

        buffer_ = buffer_.mid( slashPos + 1 + len );
        slashPos = buffer_.indexOf( "/" );
        len = buffer_.left( slashPos ).toInt();
    }
}


/*!
 * Rejects a client connection.
 */
void ChatServer::reject( QTcpSocket* client, const QString& reason ) const {
    client->write( reason.toLocal8Bit() );
    client->flush();
    client->disconnect();
    client->close();
}


/*!
 * Notifies other clients that the sender has disconnected.
 */
void ChatServer::disconnectClient() {
    auto from = static_cast<QTcpSocket*>( sender() );
    sendToClients( "/<b><font color=\"#442222\">" + clientSockets_[ from ] + " disconnected.</font></b>", from );
    clientSockets_.remove( from );
}