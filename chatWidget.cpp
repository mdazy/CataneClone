#include "chatWidget.h"
#include "messenger.h"

#include <QtCore/QTime>

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

ChatWidget::ChatWidget( Messenger* messenger ) : QWidget( Q_NULLPTR ), messenger_( messenger ) {
    auto vl = new QVBoxLayout( this );

    // read-only text view
    textView_ = new QTextEdit();
    textView_->setObjectName( "text view" );
    textView_->setReadOnly( true );
    textView_->setFocusPolicy( Qt::NoFocus );
    textView_->setAcceptRichText( false );
    vl->addWidget( textView_ );

    // input field
    inputField_ = new QLineEdit();
    inputField_->setObjectName( "input field" );
    vl->addWidget( inputField_ );
    connect( inputField_, &QLineEdit::returnPressed, this, &ChatWidget::sendText );

    connect( messenger_, &Messenger::chatMessage, this, &ChatWidget::logText );
}


void ChatWidget::sendText() {
    auto text = inputField_->text();
    inputField_->clear();
    if( text.isEmpty() ) {
        return;
    }

    logText( text );

    text = "UnspecifiedNickname: " + text;

    messenger_->sendChatMessage( text );
}


void ChatWidget::logText( const QString& text ) {
    QString time = QTime::currentTime().toString( Qt::DefaultLocaleShortDate ) + " - ";
    if( text.startsWith( "/" ) ) {
        textView_->append( time + text.right( text.length() - 1 ) );
    } else {
        textView_->append( "" );
        textView_->insertPlainText( time + text );
    }
}
