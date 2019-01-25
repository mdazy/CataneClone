#include "resourceSelectors.h"

#include "board.h"
#include "game.h"

#include <QtGui/QCloseEvent>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>


using namespace std;


/**/


ResourceSelector::ResourceSelector( QWidget* parent ) : QWidget( parent ), nbResources_( 0 ) {
    layout_ = new QGridLayout( this );
    for( int i = 0 ; i < Hex::Desert; i++ ) {
        layout_->addWidget( new QLabel( Hex::typeName[ i ][ 0 ].toUpper() + Hex::typeName[ i ].mid( 1 ) ), i, 0 );
        spin_[ i ] = new QSpinBox();
        spin_[ i ]->setMinimum( 0 );
        layout_->addWidget( spin_[ i ], i, 1 );
        connect( spin_[ i ], SIGNAL( valueChanged( int ) ), this, SLOT( updateLimits() ) );
        connect( spin_[ i ], SIGNAL( valueChanged( int ) ), this, SIGNAL( selectionChanged() ) );
    }
}


ResourceSelector::~ResourceSelector() {    
}


void ResourceSelector::setTotal( int total ) {
    nbResources_ = total;
}


void ResourceSelector::setMaxima( const std::vector<int>& maxima ) {
    if( maxima_.size() > 0 ) {
        return;
    }
    maxima_ = maxima;
    for( int i = 0; i < Hex::Desert; i++ ) {
        layout_->addWidget( new QLabel( QString( " / %1" ).arg( maxima[ i ] ) ), i, 2 ) ;
    }
    updateLimits();
}


void ResourceSelector::updateLimits() {
    int totalSelected = 0;
    for( int i = 0; i < Hex::Desert; i++ ) {
        totalSelected += spin_[ i ]->value();
    }
    if( maxima_.size() > 0 ) {
        for( int i = 0; i < Hex::Desert; i++ ) {
            spin_[ i ]->setMaximum( min( maxima_[ i ], spin_[ i ]->value() + nbResources_ - totalSelected ) );
        }
    } else if( nbResources_ > 0 ) {
        for( int i = 0; i < Hex::Desert; i++ ) {
            spin_[ i ]->setMaximum( spin_[ i ]->value() + nbResources_ - totalSelected );
        }        
    }
}


vector<int> ResourceSelector::selection() const {
    vector<int> result;
    for( int i = 0; i < Hex::Desert; i++ ) {
        result.push_back( spin_[ i ]->value() );
    }
    return result;
}


/**/


MaxedSelector::MaxedSelector( QWidget* parent ) : QDialog( parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint ) {
    setAttribute( Qt::WA_DeleteOnClose );
    auto l = new QVBoxLayout( this );
    info_ = new QLabel();
    l->addWidget( info_ );
    selector_ = new ResourceSelector();
    l->addWidget( selector_ );
    auto b = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( b, SIGNAL( accepted() ), this, SLOT( accept() ) );
    l->addWidget( b );
    OKButton_ = b->button( QDialogButtonBox::Ok );
    connect( selector_, SIGNAL( selectionChanged() ), this, SLOT( updateOKButton() ) );
    updateOKButton();
}


MaxedSelector::~MaxedSelector() {
}


void MaxedSelector::closeEvent( QCloseEvent* e ) {
    e->ignore();
}


void MaxedSelector::updateOKButton() {
    auto selection = selector_->selection();
    int total = accumulate( selection.begin(), selection.end(), 0 );
    OKButton_->setEnabled( total == max_ );
}


void MaxedSelector::accept() {
    auto selection = selector_->selection();
    int total = accumulate( selection.begin(), selection.end(), 0 );
    if( total < max_ ) {
        return;
    }
    doAccept();
    QDialog::accept();
}


/**/


DiscardSelector::DiscardSelector( Player* p, QWidget* parent ) : MaxedSelector( parent ), p_( p ) {
    max_ = p_->nbResourceCards() / 2;
    selector_->setTotal( max_ );
    selector_->setMaxima( p->resources_ );
    info_->setText( QString( "Player %1, discard %2 resource cards" ).arg( p_->number_ + 1 ).arg( max_ ) );
}


DiscardSelector::~DiscardSelector() {
}


void DiscardSelector::doAccept() {
    auto selection = selector_->selection();
    emit selected( p_, selection );   
}


/**/


NumberSelector::NumberSelector( int max, QWidget* parent ) : MaxedSelector( parent ) {
    max_ = max;
    selector_->setTotal( max_ );
    info_->setText( QString( "Select %1 resource%2" ).arg( max_ ).arg( max > 1 ? "s" : "" ) );
}


NumberSelector::~NumberSelector() {
}


void NumberSelector::doAccept() {
    emit selected( selector_->selection() );
}


/**/


TradeSelector::TradeSelector( Player* p, QWidget* parent ) : QDialog( parent ) {
    setAttribute( Qt::WA_DeleteOnClose );
    auto l = new QVBoxLayout( this );
    setLayout( l );
    l->addWidget( new QLabel( "Not implemented yet" ) );
}


TradeSelector::~TradeSelector() {
}
