/*
 *   This file is part of the Okteta Kasten Framework, made within the KDE community.
 *
 *   Copyright 2009, 2010, 2011  Alex Richardson <alex.richardson@gmx.de>
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
#ifndef FLOATDATAINFORMATION_H_
#define FLOATDATAINFORMATION_H_

#include "primitivedatainformation.h"

class FloatDataInformation: public PrimitiveDataInformation
{
PRIMITIVEDATAINFORMATION_SUBCLASS_CONSTRUCTORS(Float,Primitive)
public:
    DATAINFORMATION_CLONE(Float)

    virtual int displayBase() const;
    virtual int size() const;
    virtual QString typeName() const;
    virtual QString valueString() const;
    virtual PrimitiveDataType type() const;
    virtual AllPrimitiveTypes value() const;
    virtual void setValue(AllPrimitiveTypes newVal);
    virtual QScriptValue valueAsQScriptValue() const;
    virtual QWidget* createEditWidget(QWidget* parent) const;
    virtual QVariant dataFromWidget(const QWidget* w) const;
    virtual void setWidgetData(QWidget* w) const;
    virtual AllPrimitiveTypes qVariantToAllPrimitiveTypes(const QVariant& value) const;
private:
    float mValue;
};

inline PrimitiveDataType FloatDataInformation::type() const
{
    return Type_Float;
}

inline int FloatDataInformation::displayBase() const
{
    return 10;
}

inline int FloatDataInformation::size() const
{
    return 32;
}

inline QString FloatDataInformation::typeName() const
{
    return i18nc("Data type", "float");
}

#endif /* FLOATDATAINFORMATION_H_ */
