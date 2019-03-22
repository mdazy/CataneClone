#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMainWindow>

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>

#include "gameView.h"
#include "game.h"
#include "chatServer.h"
#include "messenger.h"

#include <vector>
#include <ctime>

#include <iostream>

using namespace std;


int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    app.setApplicationName( "Qatane" );
    app.setApplicationVersion( "0.8" );

    // arguments parsing
    QCommandLineParser parser;
    parser.setApplicationDescription( "Qatane is a homemade version of Settlers of Catan, for personal use only." );
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption serverOption(
        QStringList() << "s" << "server",
        "Start the game as server"
    );
    parser.addOption( serverOption );

    QCommandLineOption portOption(
        QStringList() << "p" << "port",
        "Port of the game server, defaults to 12345",
        "port",
        "12345"
    );
    parser.addOption( portOption );

    parser.process( app );

    bool server = parser.isSet( serverOption );
    int port = parser.value( portOption ).toInt();

    cerr << ( server ? "server" : "client" ) << " on port " << port << endl;

    // setup chat server
    if( server ) {
        new ChatServer( port, &app );
    }

    QMainWindow mainWindow;

    srand( time( 0 ) );

    Messenger messenger( port, &app );
    Game game( &messenger, server );

    auto view = new GameView( &game, &messenger );
    view->setMinimumSize( 500, 500 );
    
    mainWindow.setCentralWidget( view );
    
    auto desktop = app.desktop();
    QRect screen = desktop->screenGeometry( desktop->primaryScreen() );
    mainWindow.move( ( screen.width() - 500 ) / 2, ( screen.height() - 500 ) / 2 );
    mainWindow.show();

    game.newGame();
    
    return app.exec();
}
