#include "userdata.h"

SearchInfo::SearchInfo(int uid, QString name,QString nick, QString desc, int sex, QString icon):
    _uid(uid), _name(name), _nick(nick), _desc(desc), _sex(sex), _icon(icon){
}

AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc):_from_uid(from_uid),_name(name),_desc(desc)
{

}
