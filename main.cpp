#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "view.h"

#include <vector>

using namespace std;


int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    QMainWindow mainWindow;

    srand( time( 0 ) );

    auto view = new View();
    Board board;
    view->board_ = &board;
    
    auto mainW = new QWidget();
    mainWindow.setCentralWidget( mainW );
    
    auto mainL = new QVBoxLayout( mainW );
    mainL->addWidget( view );
    
    auto exitB = new QPushButton( "Quit" );
    exitB->connect( exitB, SIGNAL( clicked() ), &app, SLOT( quit() ) );
    mainL->addWidget( exitB );
    
    mainWindow.resize( 500, 500 );
    auto desktop = app.desktop();
    mainWindow.move( ( desktop->width() - 500 ) / 2, ( desktop->height() - 500 ) / 2 );
    mainWindow.show();
    return app.exec();
}
