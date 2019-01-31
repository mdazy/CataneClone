#include "resourceSelectors.h"

#include "board.h"
#include "game.h"

#include <QtGui/QValidator>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>


using namespace std;


/**/


/*
 * Custom spinbox that enables up and down only if stepping does not
 * underflow the minimum value or overflow the maximum value.
 */
class MyQSpinBox : public QSpinBox {
public:
    MyQSpinBox( QWidget* parent = Q_NULLPTR ) : QSpinBox( parent ) {}
    virtual ~MyQSpinBox() {}

protected:
    StepEnabled stepEnabled() const {
        StepEnabled result = StepNone;
        if( value() + singleStep() <= maximum() ) {
            result |= StepUpEnabled;
        }
        if( value() - singleStep() >= minimum() ) {
            result |= StepDownEnabled;
        }
        return result;
    }

    void fixup(QString& input) const override {
        int value = input.toInt();
        value = min( value, maximum() );
        value -= value % singleStep();
        input = QString::number( value );
    }

    QValidator::State validate( QString& input, int& ) const override {
        bool ok = false;
        int value = input.toInt( &ok );
        if( !ok ) {
            return QValidator::Invalid;
        }
        if( value % singleStep() == 0 ) {
            return QValidator::Acceptable;
        }
        return QValidator::Intermediate;
    }

};


/**/


ResourceSelector::ResourceSelector( QWidget* parent ) : QWidget( parent ), nbResources_( 0 ) {
    layout_ = new QGridLayout( this );
    for( int i = 0 ; i < Hex::Desert; i++ ) {
        layout_->addWidget( new QLabel( Hex::typeName[ i ][ 0 ].toUpper() + Hex::typeName[ i ].mid( 1 ) ), i, 0 );
        spin_[ i ] = new MyQSpinBox();
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


void ResourceSelector::setSteps( const std::vector<int>& steps ) {
    for( int i = 0; i < Hex::Desert; i++ ) {
        spin_[ i ]->setSingleStep( steps[ i ] );
        layout_->addWidget( new QLabel( QString( "(%1)" ).arg( steps[ i ] ) ), i, 3 ) ;
    }
}


void ResourceSelector::updateLimits() {
    int totalSelected = 0;
    for( int i = 0; i < Hex::Desert; i++ ) {
        totalSelected += spin_[ i ]->value();
    }
    if( maxima_.size() > 0 ) {
        for( int i = 0; i < Hex::Desert; i++ ) {
            if( nbResources_ > 0 ) {
                spin_[ i ]->setMaximum( min( maxima_[ i ], spin_[ i ]->value() + nbResources_ - totalSelected ) );
            } else {
                spin_[ i ]->setMaximum( maxima_[ i ] );
            }
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
    selectorLayout_ = new QGridLayout;
    l->addLayout( selectorLayout_ );
    info_ = new QLabel();
    selectorLayout_->addWidget( info_, 0, 1 );
    selector_ = new ResourceSelector();
    selectorLayout_->addWidget( selector_, 1, 1 );
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


TradeSelector::TradeSelector( Player* p, QWidget* parent ) : MaxedSelector( parent ), p_( p ) {
    selectorLayout_->addWidget( new QLabel( "Select resources to sell"), 0, 0 );
    fromSel_ = new ResourceSelector( this );
    fromSel_->setMaxima( p->resources_ );
    fromSel_->setSteps( p->cardCosts() );
    selectorLayout_->addWidget( fromSel_, 1, 0 );
    updateMax();
    connect( fromSel_, SIGNAL( selectionChanged() ), this, SLOT( updateMax() ) );
}


TradeSelector::~TradeSelector() {
}


void TradeSelector::updateMax() {
    max_ = 0;
    const auto& cards = fromSel_->selection();
    for( int i = 0; i < p_->cardCosts().size(); i++  ) {
        max_ += cards[ i ] / p_->cardCosts()[ i ];
    }
    info_->setText( QString( "Select %1 card%2 to buy" ).arg( max_ ).arg ( max_ > 1 ? "s" : "" ) );
    updateOKButton();
}


void TradeSelector::doAccept() {
    emit selected( fromSel_->selection(), selector_->selection() );
}