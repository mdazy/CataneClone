#pragma once

#include <QtWidgets/QWidget>

class Messenger;

class QLineEdit;
class QTextEdit;

class ChatWidget : public QWidget {
    Q_OBJECT;
public:
    ChatWidget( Messenger* messenger );
    virtual ~ChatWidget() = default;

private slots:
    void sendText();
    void logText( const QString& text );

private:
    Messenger* messenger_;
    QLineEdit* inputField_;
    QTextEdit* textView_;
};