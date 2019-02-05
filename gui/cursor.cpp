/*
    This file is part of the Okteta Gui library, made within the KDE community.

    Copyright 2003 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "cursor.hpp"

namespace Okteta {

Cursor::Cursor() = default;

Cursor::~Cursor() = default;

void Cursor::setSize(PixelX Width, PixelY Height, qreal devicePixelRatio)
{
    const int scaledWidth = Width * devicePixelRatio;
    const int scaledHeight = Height * devicePixelRatio;
    if (scaledWidth != OnPixmap.width() || scaledHeight != OnPixmap.height()) {
        OnPixmap = QPixmap(scaledWidth, scaledHeight);
        OffPixmap = QPixmap(scaledWidth, scaledHeight);
    }
    OnPixmap.setDevicePixelRatio(devicePixelRatio);
    OffPixmap.setDevicePixelRatio(devicePixelRatio);
}

void Cursor::setShape(PixelX X, PixelX W, qreal devicePixelRatio)
{
    CursorX = X;
    ShapeX = X * devicePixelRatio;
    ShapeW = W * devicePixelRatio;
}

}