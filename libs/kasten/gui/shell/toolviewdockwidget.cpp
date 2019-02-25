/*
    This file is part of the Kasten Framework, made within the KDE community.

    Copyright 2008,2019 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "toolviewdockwidget.hpp"
#include "toolviewdockwidget_p.hpp"

// lib
#include <abstracttoolview.hpp>

namespace Kasten {

ToolViewDockWidget::ToolViewDockWidget(AbstractToolView* toolView, QWidget* parent)
    : QDockWidget(toolView->title(), parent)
    , d_ptr(new ToolViewDockWidgetPrivate(toolView))
{
    Q_D(ToolViewDockWidget);

    d->init(this);
}

ToolViewDockWidget::~ToolViewDockWidget() = default;

AbstractToolView* ToolViewDockWidget::toolView() const
{
    Q_D(const ToolViewDockWidget);

    return d->toolView();
}

bool ToolViewDockWidget::isShown() const
{
    Q_D(const ToolViewDockWidget);

    return d->isShown();
}

}
