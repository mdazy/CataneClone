#include "resourceSelectors.h"

#include "board.h"
#include "game.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>


using namespace std;


/**/


ResourceSelector::ResourceSelector( int nbResources, QWidget* parent ) : QWidget( parent ), nbResources_( nbResources ) {
    layout_ = new QGridLayout( this );
    for( int i = 0 ; i < Hex::Desert; i++ ) {
        layout_->addWidget( new QLabel( Hex::typeName[ i ] ), i, 0 );
        spin_[ i ] = new QSpinBox();
        spin_[ i ]->setMinimum( 0 );
        layout_->addWidget( spin_[ i ], i, 1 );
        connect( spin_[ i ], SIGNAL( valueChanged( int ) ), this, SLOT( updateLimits() ) );
        connect( spin_[ i ], SIGNAL( valueChanged( int ) ), this, SIGNAL( selectionChanged() ) );
    }
}


ResourceSelector::~ResourceSelector() {    
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


DiscardSelector::DiscardSelector( Player* p, QWidget* parent ) : QDialog( parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint ), p_( p ) {
    // TODO: prevent close with Alt-F4
    setAttribute( Qt::WA_DeleteOnClose );
    auto l = new QVBoxLayout( this );
    nbCards_ = p->nbResourceCards() / 2;
    l->addWidget( new QLabel( QString( "Player %1, discard %2 resource cards" ).arg( p_->number_ + 1 ).arg( nbCards_ ) ) );
    selector_ = new ResourceSelector( nbCards_ );
    selector_->setMaxima( p->resources_ );
    l->addWidget( selector_ );
    auto b = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( b, SIGNAL( accepted() ), this, SLOT( accept() ) );
    l->addWidget( b );
    OKButton_ = b->button( QDialogButtonBox::Ok );
    connect( selector_, SIGNAL( selectionChanged() ), this, SLOT( updateOKButton() ) );
}


DiscardSelector::~DiscardSelector() {
}


void DiscardSelector::updateOKButton() {
    auto selection = selector_->selection();
    int total = accumulate( selection.begin(), selection.end(), 0 );
    OKButton_->setEnabled( total == nbCards_ );
}


void DiscardSelector::accept() {
    auto selection = selector_->selection();
    int total = accumulate( selection.begin(), selection.end(), 0 );
    if( total < nbCards_ ) {
        return;
    }
    emit selected( p_, selection );
    QDialog::accept();
}