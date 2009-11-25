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

#ifndef ENUMDEFINITION_H_
#define ENUMDEFINITION_H_
#include "../allprimitivetypes.h"
//Qt core
#include <QMap>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QString>
#include <QList>

class EnumDefinition: public QSharedData
{
public:
    typedef QSharedDataPointer<EnumDefinition> Ptr;
    EnumDefinition(const QMap<AllPrimitiveTypes, QString> values, QString name,
            PrimitiveDataType type) :
        QSharedData(), mName(name), mValues(values), mType(type)
    {
    }
    EnumDefinition(const EnumDefinition& e) :
        QSharedData(e), mValues(e.mValues), mType(e.mType)
    {
    }

    const QMap<AllPrimitiveTypes, QString>& values() const
    {
        return mValues;
    }
    const AllPrimitiveTypes key(QString& value) const
    {
        return mValues.key(value);
    }
    const QString value(AllPrimitiveTypes key) const
    {
        return mValues.value(key);
    }
    PrimitiveDataType type() const
    {
        return mType;
    }
    const QString& name() const
    {
        return mName;
    }
    static EnumDefinition::Ptr find(const QString& defName, const QList<
            EnumDefinition::Ptr>& list)
    {
        for (int i = 0; i < list.length(); ++i)
        {
            EnumDefinition::Ptr def = list.at(i);
            if (def->name() == defName)
            {
                return def;
            }
        }
        return EnumDefinition::Ptr(NULL);
    }
protected:
    const QString mName;
    const QMap<AllPrimitiveTypes, QString> mValues;
    const PrimitiveDataType mType;
};

#endif /* ENUMDEFINITION_H_ */
