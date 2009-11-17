/*
    This file is part of the Okteta Kasten module, part of the KDE project.

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

#include "sint64editor.h"

// C++
#include <limits>


SInt64Editor::SInt64Editor( QWidget* parent )
  : QLineEdit( parent )
{
//     const qint64 int64Max = std::numeric_limits<qint64>::max();
//     const qint64 int64Min = std::numeric_limits<qint64>::min();

//     setRange( int64Min, int64Max );
}

void SInt64Editor::setData( SInt64 data )
{
    setText( data.toString() );
}

SInt64 SInt64Editor::data() const
{
//     interpretText();
    return text().toLongLong();
}

SInt64Editor::~SInt64Editor() {}