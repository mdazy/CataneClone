#include <QtGui/QPainter>
#include <QtGui/QPolygonF>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <cmath>

using namespace std;

const float degToRad = 3.1415926 / 180.0;

class View : public QWidget {
public:
    View( QWidget* parent = Q_NULLPTR ) : QWidget( parent ) {
    }
    virtual ~View() {
    }
    
    void paintEvent( QPaintEvent* event ) override {
        QPainter p( this );
        p.setBrush( Qt::red );
        
        QPolygonF hex;
        int radius = min( width(), height() ) / 2;
        for( int i = 0; i < 6; i++ ) {
            hex << QPointF( radius * cos( i * 60 * degToRad ), radius * sin( i * 60 * degToRad ) );
        }
        
        p.drawPolygon( hex.translated( QPointF( width() / 2, height() / 2 ) ) );
    }
    
};

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
