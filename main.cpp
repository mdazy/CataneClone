#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "view.h"


int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    QMainWindow mainWindow;
    
    auto mainW = new QWidget();
    mainWindow.setCentralWidget( mainW );
    
    auto mainL = new QVBoxLayout( mainW );
    auto view = new View();
    mainL->addWidget( view );
    
    auto exitB = new QPushButton( "Quit" );
    exitB->connect( exitB, SIGNAL( clicked() ), &app, SLOT( quit() ) );
    mainL->addWidget( exitB );
    
    mainWindow.show();
    return app.exec();
}
