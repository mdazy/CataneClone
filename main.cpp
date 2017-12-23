#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    QMainWindow mainWindow;
    
    auto mainW = new QWidget();
    mainWindow.setCentralWidget( mainW );
    
    auto mainL = new QVBoxLayout( mainW );
    mainL->addWidget( new QLabel( "Hello, world." ) );
    
    auto exitB = new QPushButton( "Quit" );
    exitB->connect( exitB, SIGNAL( clicked() ), &app, SLOT( quit() ) );
    mainL->addWidget( exitB );
    
    mainWindow.show();
    return app.exec();
}
