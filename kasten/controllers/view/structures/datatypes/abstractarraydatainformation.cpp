/*
 *   This file is part of the Okteta Kasten module, part of the KDE project.
 *
 *   Copyright 2009 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
#include "abstractarraydatainformation.h"
#include "primitivedatainformation.h"
#include "dynamiclengtharraydatainformation.h"
#include "staticlengtharraydatainformation.h"

QString AbstractArrayDataInformation::getTypeName() const
{
    if (!hasChildren())
        return i18n("Empty array");

    DataInformation* data = childAt(0);
    if (dynamic_cast<PrimitiveDataInformation*> (data))
    {
        //don't show name of child
        return i18nc("array type then length", "%1[%2]", data->getTypeName(),
                childCount());
    }
    return i18nc("subelem type then array name and length", "%1[%2] (%3)",
            data->getName(), childCount(), data->getTypeName(), data->getName());
}

AbstractArrayDataInformation::AbstractArrayDataInformation(QString name, int index,
        DataInformation* parent) :
    DataInformationWithChildren(name, index, parent)
{
}

AbstractArrayDataInformation::AbstractArrayDataInformation(
        const AbstractArrayDataInformation& d) :
    DataInformationWithChildren(d)
{
}

AbstractArrayDataInformation::~AbstractArrayDataInformation()
{
}

Okteta::Size AbstractArrayDataInformation::offset(unsigned int index) const
{
    if (index >= childCount())
        return 0;
    Okteta::Size offset = 0;
    //sum size of elements up to index
    for (unsigned int i = 0; i < index; ++i)
    {
        offset += childAt(i)->getSize() / 8;
    }
    return offset;
}
int AbstractArrayDataInformation::getSize() const
{
    int size = 0;
    for (unsigned int i = 0; i < childCount(); ++i)
    {
        size += childAt(i)->getSize();
    }
    return size;
}