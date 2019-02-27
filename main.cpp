#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMainWindow>

#include "gameView.h"
#include "game.h"
#include "messenger.h"

#include <vector>
#include <ctime>

using namespace std;


int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    QMainWindow mainWindow;

    srand( time( 0 ) );

    Messenger messenger;
    Game game( &messenger );

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
