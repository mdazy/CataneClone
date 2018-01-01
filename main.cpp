#include <QtWidgets/QApplication>
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

    auto view = new View();
    Board board( 7, 7 );
    view->board_ = &board;
    
    auto mainW = new QWidget();
    mainWindow.setCentralWidget( mainW );
    
    auto mainL = new QVBoxLayout( mainW );
    mainL->addWidget( view );
    
    auto exitB = new QPushButton( "Quit" );
    exitB->connect( exitB, SIGNAL( clicked() ), &app, SLOT( quit() ) );
    mainL->addWidget( exitB );
    
    mainWindow.show();
    return app.exec();
}
