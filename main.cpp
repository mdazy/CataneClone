#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "view.h"

#include <vector>
#include <ctime>

using namespace std;


int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    QMainWindow mainWindow;

    srand( time( 0 ) );

    auto view = new View();
    view->setMinimumSize( 500, 500 );
    Board board;
    view->board_ = &board;
    
    auto mainW = new QWidget();
    mainWindow.setCentralWidget( mainW );
    
    auto mainL = new QVBoxLayout( mainW );
    mainL->addWidget( view );
    
    auto desktop = app.desktop();
    mainWindow.move( ( desktop->width() - 500 ) / 2, ( desktop->height() - 500 ) / 2 );
    mainWindow.show();
    return app.exec();
}
