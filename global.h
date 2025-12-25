#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include "QRegularExpression"

//刷新操作
extern std::function<void(QWidget*)> repolish;

#endif // GLOBAL_H
