/*
    This file is part of the Kasten Framework, made within the KDE community.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "documentstoolviewfactory.hpp"

// lib
#include "documentstoolview.hpp"
#include "documentstool.hpp"
// KF5
#include <KLocalizedString>

namespace Kasten {

DocumentsToolViewFactory::DocumentsToolViewFactory() = default;

DocumentsToolViewFactory::~DocumentsToolViewFactory() = default;

QString DocumentsToolViewFactory::iconName() const { return QStringLiteral("view-history"); }
QString DocumentsToolViewFactory::title()    const { return i18nc("@title:window", "Documents"); }
QString DocumentsToolViewFactory::id()       const { return QStringLiteral("org.kde.kasten.DocumentsToolView"); }
SidePosition DocumentsToolViewFactory::defaultPosition() const { return LeftSidePosition; }

AbstractToolView* DocumentsToolViewFactory::create(AbstractTool* tool) const
{
    return new DocumentsToolView(qobject_cast<DocumentsTool*>(tool));
}

}