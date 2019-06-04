/*
    This file is part of the Kasten Framework, made within the KDE community.

    Copyright 2009 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KASTEN_IF_REPLACEUSERQUERYABLE_HPP
#define KASTEN_IF_REPLACEUSERQUERYABLE_HPP

// libfinddialog
#include <kasten/okteta/finddirection.hpp>
// Qt
#include <QtPlugin>

namespace Kasten {

enum ReplaceBehaviour
{
    ReplaceAll,
    SkipCurrent,
    ReplaceCurrent,
    CancelReplacing
};

namespace If {

class ReplaceUserQueryable
{
public:
    virtual ~ReplaceUserQueryable();

public: // API to be implemented
    virtual void queryContinue(FindDirection direction, int noOfReplacements) = 0;
    virtual void queryReplaceCurrent() = 0;

public: // signals
    virtual void queryContinueFinished(bool result) = 0;
    virtual void queryReplaceCurrentFinished(ReplaceBehaviour result) = 0;
};

inline ReplaceUserQueryable::~ReplaceUserQueryable() = default;

}
}

Q_DECLARE_INTERFACE(Kasten::If::ReplaceUserQueryable, "org.kde.kasten.if.replaceuserqueryable/1.0")

#endif
