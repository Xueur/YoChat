#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H
#include <QLineEdit>
#include <QDebug>

class CustomizeEdit: public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit(QWidget* parent);
    void SetMaxLength(int length);
protected:
    void focusOutEvent(QFocusEvent* event);
private:
    void limitTextLength(QString text);
    int _max_len;
signals:
    void sig_focus_out();
    void sig_clear_edit();
};

#endif // CUSTOMIZEEDIT_H
