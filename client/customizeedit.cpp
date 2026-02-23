#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget* parent):QLineEdit(parent) {
    QAction *searchAction = new QAction(this);
    searchAction->setIcon(QIcon(":/res/search.png"));
    this->addAction(searchAction,QLineEdit::LeadingPosition);
    this->setPlaceholderText(QStringLiteral("搜索"));

    QAction *clearAction = new QAction(this);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    // 初始时不显示清除图标
    this->addAction(clearAction, QLineEdit::TrailingPosition);
    connect(this, &QLineEdit::textChanged, [clearAction](const QString &text) {
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        } else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
        }
    });
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        this->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
        this->clearFocus();
        emit sig_clear_edit();
    });
    this->SetMaxLength(30);

    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::SetMaxLength(int length)
{
    _max_len = length;
}

void CustomizeEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    emit sig_focus_out();
}

void CustomizeEdit::limitTextLength(QString text)
{
    if (_max_len <= 0) {
        return;
    }
    QByteArray byteArray = text.toUtf8();
    if (byteArray.size() > _max_len) {
        byteArray = byteArray.left(_max_len);
        this->setText(QString::fromUtf8(byteArray));
    }
}
