/*
    Copyright (C) 2015 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef PROFILE_H
#define PROFILE_H

#include "toxcore.h"
#include "toxav.h"
#include "friend.h"
#include "group.h"

template<class K, class V> class QHash;

class Profile : public ToxCore
{
    Q_OBJECT

public:
    QHash<int, Friend> friendsList;
    QHash<int, Group*> groupList;
    ToxAv toxav;
};


#endif // PROFILE_H
