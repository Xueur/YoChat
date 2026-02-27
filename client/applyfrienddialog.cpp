#include "applyfrienddialog.h"
#include "ui_applyfrienddialog.h"
#include "clickedlabel.h"
#include "friendlabel.h"
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include "usermgr.h"
#include "tcpmgr.h"

ApplyFriendDialog::ApplyFriendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplyFriendDialog),_label_point(2,6)
{
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriendDialog");
    this->setModal(true);
    ui->name_edit->setPlaceholderText(tr("DreamsLentenbe"));
    ui->label_edit->setPlaceholderText("搜索、添加标签");
    ui->back_edit->setPlaceholderText("可爱的淇酱");

    ui->label_edit->SetMaxLength(21);
    ui->label_edit->move(2, 2);
    ui->label_edit->setFixedHeight(20);
    ui->label_edit->setMaxLength(10);
    ui->input_tip_widget->hide();

    _tip_cur_point = QPoint(5, 5);

    _tip_data = { "同学","家人","菜鸟教程","C++ Primer","Rust",
                 "父与子学Python","nodejs","golang",
                 "游戏伙伴","金融投资","微信读书","拼多多拼友" };
    connect(ui->more_label, &ClickedOnceLabel::clicked, this, &ApplyFriendDialog::ShowMoreLabel);
    InitTipLabels();
    //链接输入标签回车事件
    connect(ui->label_edit, &CustomizeEdit::returnPressed, this, &ApplyFriendDialog::SlotLabelEnter);
    connect(ui->label_edit, &CustomizeEdit::textChanged, this, &ApplyFriendDialog::SlotLabelTextChange);
    connect(ui->label_edit, &CustomizeEdit::editingFinished, this, &ApplyFriendDialog::SlotLabelEditFinished);
    connect(ui->tip_label, &ClickedOnceLabel::clicked, this, &ApplyFriendDialog::SlotAddFriendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->SetState("normal","hover","press");
    ui->cancel_btn->SetState("normal","hover","press");
    //连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriendDialog::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriendDialog::SlotApplySure);
}

ApplyFriendDialog::~ApplyFriendDialog()
{
    qDebug()<< "ApplyFriendDialog destruct";
    delete ui;
}

void ApplyFriendDialog::InitTipLabels()
{
    int lines = 1;
    for(int i = 0; i < (int)_tip_data.size(); i++){

        auto* label = new ClickedLabel(ui->label_list);
        label->SetState("normal", "hover", "pressedit", "selectedit_normal",
                     "selectedit_hover", "selectedit_pressedit");
        label->setObjectName("tipslabel");
        label->setText(_tip_data[i]);
        connect(label, &ClickedLabel::clicked, this, &ApplyFriendDialog::SlotChangeFriendLabelByTip);
        QFontMetrics fontMetrics(label->font()); // 获取QLabel控件的字体信息
        int textwidgetth = fontMetrics.horizontalAdvance(label->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textwidgetth + tip_offset > ui->label_list->width()) {
            lines++;
            if (lines > 2) {
                delete label;
                return;
            }

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

        auto next_point = _tip_cur_point;

        AddTipLabels(label, _tip_cur_point,next_point, textwidgetth, textHeight);

        _tip_cur_point = next_point;
    }

}


void ApplyFriendDialog::AddTipLabels(ClickedLabel* label, QPoint cur_point, QPoint& next_point, int text_width, int text_height)
{
    Q_UNUSED(text_height);
    label->move(cur_point);
    label->show();
    _add_labels.insert(label->text(), label);
    _add_label_keys.push_back(label->text());
    next_point.setX(label->pos().x() + text_width + 15);
    next_point.setY(label->pos().y());
}

bool ApplyFriendDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void ApplyFriendDialog::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    // auto applyname = UserMgr::getInstance()->GetName();
    auto applyname = "Xueur";
    auto bakname = si->_name;
    ui->name_edit->setText(applyname);
    ui->back_edit->setText(bakname);
}

void ApplyFriendDialog::ShowMoreLabel()
{
    qDebug()<< "receive more label clickedit";
    ui->more_label_widget->hide();

    ui->label_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textwidgetth;
    int textHeight;
    //重拍现有的label
    for(auto & addedit_key : _add_label_keys){
        auto addedit_label = _add_labels[addedit_key];

        QFontMetrics fontMetrics(addedit_label->font()); // 获取QLabel控件的字体信息
        textwidgetth = fontMetrics.horizontalAdvance(addedit_label->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(_tip_cur_point.x() +textwidgetth + tip_offset > ui->label_list->width()){
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y()+textHeight+15);
        }
        addedit_label->move(_tip_cur_point);

        next_point.setX(addedit_label->pos().x() + textwidgetth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;

    }

    //添加未添加的
    for(int i = 0; i < (int)_tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }

        auto* label = new ClickedLabel(ui->label_list);
        label->SetState("normal", "hover", "pressedit", "selectedit_normal",
                     "selectedit_hover", "selectedit_pressedit");
        label->setObjectName("tipslabel");
        label->setText(_tip_data[i]);
        connect(label, &ClickedLabel::clicked, this, &ApplyFriendDialog::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(label->font()); // 获取QLabel控件的字体信息
        int textwidgetth = fontMetrics.horizontalAdvance(label->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textwidgetth + tip_offset > ui->label_list->width()) {

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

        next_point = _tip_cur_point;

        AddTipLabels(label, _tip_cur_point, next_point, textwidgetth, textHeight);

        _tip_cur_point = next_point;

    }

    int diff_height = next_point.y() + textHeight + tip_offset - ui->label_list->height();
    ui->label_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    //qDebug()<<"after resize ui->label_list size is " <<  ui->label_list->size();
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+diff_height);
}

void ApplyFriendDialog::resetLabels()
{
    auto max_widgetth = ui->grid_widget->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++){
        if( _label_point.x() + iter.value()->width() > max_widgetth) {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if(_friend_labels.isEmpty()){
        ui->label_edit->move(_label_point);
        return;
    }

    if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width()){
        ui->label_edit->move(2,_label_point.y()+label_height+6);
    }else{
        ui->label_edit->move(_label_point);
    }
}

void ApplyFriendDialog::addLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end()) {
        ui->label_edit->clear();
        return;
    }

    auto tmplabel = new FriendLabel(ui->grid_widget);
    tmplabel->SetText(name);
    tmplabel->setObjectName("FriendLabel");

    auto max_widgetth = ui->grid_widget->width();
    if (_label_point.x() + tmplabel->width() > max_widgetth) {
        _label_point.setY(_label_point.y() + tmplabel->height() + 6);
        _label_point.setX(2);
    }
    else {

    }
    tmplabel->move(_label_point);
    tmplabel->show();
    _friend_labels[tmplabel->Text()] = tmplabel;
    _friend_label_keys.push_back(tmplabel->Text());

    connect(tmplabel, &FriendLabel::sig_close, this, &ApplyFriendDialog::SlotRemoveFriendLabel);

    _label_point.setX(_label_point.x() + tmplabel->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width()) {
        ui->label_edit->move(2, _label_point.y() + tmplabel->height() + 2);
    }
    else {
        ui->label_edit->move(_label_point);
    }

    ui->label_edit->clear();

    if (ui->grid_widget->height() < _label_point.y() + tmplabel->height() + 2) {
        ui->grid_widget->setFixedHeight(_label_point.y() + tmplabel->height() * 2 + 2);
    }
}

void ApplyFriendDialog::SlotLabelEnter()
{
    if(ui->label_edit->text().isEmpty()){
        return;
    }

    auto text = ui->label_edit->text();

    addLabel(ui->label_edit->text());

    ui->input_tip_widget->hide();

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }
    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* label = new ClickedLabel(ui->label_list);
    label->SetState("normal", "hover", "pressedit", "selectedit_normal",
                 "selectedit_hover", "selectedit_pressedit");
    label->setObjectName("tipslabel");
    label->setText(text);
    connect(label, &ClickedLabel::clicked, this, &ApplyFriendDialog::SlotChangeFriendLabelByTip);
    qDebug() << "ui->label_list->width() is " << ui->label_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(label->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(label->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textwidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->label_list->width()) {
        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
    }

    auto next_point = _tip_cur_point;

    AddTipLabels(label, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->label_list->height();
    ui->label_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    label->SetCurState(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDialog::SlotRemoveFriendLabel(QString name)
{
    qDebug() << "receive close signal";

    _label_point.setX(2);
    _label_point.setY(6);

    auto find_iter = _friend_labels.find(name);

    if(find_iter == _friend_labels.end()){
        return;
    }

    auto find_key = _friend_label_keys.end();
    for(auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end();
         iter++){
        if(*iter == name){
            find_key = iter;
            break;
        }
    }

    if(find_key != _friend_label_keys.end()){
        _friend_label_keys.erase(find_key);
    }


    delete find_iter.value();

    _friend_labels.erase(find_iter);

    resetLabels();

    auto find_add = _add_labels.find(name);
    if(find_add == _add_labels.end()){
        return;
    }

    find_add.value()->ResetNormalState();
}

//点击标已有签添加或删除新联系人的标签
void ApplyFriendDialog::SlotChangeFriendLabelByTip(QString labeltext, ClickLbState state)
{
    auto find_iter = _add_labels.find(labeltext);
    if(find_iter == _add_labels.end()){
        return;
    }

    if(state == ClickLbState::Selected){
        //编写添加逻辑
        addLabel(labeltext);
        return;
    }

    if(state == ClickLbState::Normal){
        //编写删除逻辑
        SlotRemoveFriendLabel(labeltext);
        return;
    }

}

void ApplyFriendDialog::SlotLabelTextChange(const QString& text)
{
    if (text.isEmpty()) {
        ui->tip_label->setText("");
        ui->input_tip_widget->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = add_prefix + text;
        ui->tip_label->setText(new_text);
        ui->input_tip_widget->show();
        return;
    }
    ui->tip_label->setText(text);
    ui->input_tip_widget->show();
}

void ApplyFriendDialog::SlotLabelEditFinished()
{
    ui->input_tip_widget->hide();
}

void ApplyFriendDialog::SlotAddFriendLabelByClickTip(QString text)
{
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* label = new ClickedLabel(ui->label_list);
    label->SetState("normal", "hover", "pressedit", "selectedit_normal",
                 "selectedit_hover", "selectedit_pressedit");
    label->setObjectName("tipslabel");
    label->setText(text);
    connect(label, &ClickedLabel::clicked, this, &ApplyFriendDialog::SlotChangeFriendLabelByTip);
    qDebug() << "ui->label_list->widgetth() is " << ui->label_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(label->font()); // 获取QLabel控件的字体信息
    int textwidgetth = fontMetrics.horizontalAdvance(label->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textwidgetth is " << textwidgetth;

    if (_tip_cur_point.x() + textwidgetth+ tip_offset+3 > ui->label_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLabels(label, _tip_cur_point, next_point, textwidgetth,textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->label_list->height();
    ui->label_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    label->SetCurState(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+ diff_height );
}

void ApplyFriendDialog::SlotApplySure()
{
    qDebug()<<"Slot Apply Sure calledit" ;
    //发送请求逻辑
    QJsonObject jsonObj;
    auto uid = UserMgr::getInstance()->GetUid();
    jsonObj["uid"] = uid;
    auto name = ui->name_edit->text();
    if(name.isEmpty()){
        name = ui->name_edit->placeholderText();
    }

    jsonObj["applyname"] = name;

    auto bakname = ui->back_edit->text();
    if(bakname.isEmpty()){
        bakname = ui->back_edit->placeholderText();
    }

    jsonObj["bakname"] = bakname;
    jsonObj["touid"] = _si->_uid;

    QJsonDocument doc(jsonObj);
    QString jsonString = doc.toJson(QJsonDocument::Indented);

    //发送tcp请求给chat server
    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_ADD_FRIEND_REQ, jsonString);
    this->hide();
    deleteLater();
}

void ApplyFriendDialog::SlotApplyCancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

