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
    vector<vector<int>> grid( 7, vector<int>( 7 ) );
    for( int x = 0; x < 7; x++ ) {
    	for( int y = 0; y < 7; y++ ) {
    		grid[ y ][ x ] = 0; // invalid
    	}
    }
    view->grid_ = &grid;
    
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
