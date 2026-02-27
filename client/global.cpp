#include "global.h"

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [](QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input) {
    QString result = input;
    int length = input.length();
    length %= 255;
    for (int i = 0; i < length; i++) {
        result[i] = QChar(static_cast<ushort>(input[i].unicode()) ^ static_cast<ushort>(length));
    }
    return result;
};

std::vector<QString>  strs ={"Hello world!",
                             "Nice to meet you!",
                             "New year，new life!",
                             "You have to love yourself!",
                             "My love is written in the wind ever since the whole world is you!"};
std::vector<QString> heads = {
    ":/res/head_1.jpg",
    ":/res/head_2.jpg",
    ":/res/head_3.jpg",
    ":/res/head_4.jpg",
    ":/res/head_5.jpg"
};
std::vector<QString> names = {
    "Qt",
    "Cpp",
    "Golang",
    "java",
    "Nodejs",
    "python",
    "rust"
};
