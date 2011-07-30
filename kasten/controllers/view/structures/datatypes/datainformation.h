/*
 *   This file is part of the Okteta Kasten Framework, made within the KDE community.
 *
 *   Copyright 2009, 2010 Alex Richardson <alex.richardson@gmx.de>
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
#ifndef DATAINFORMATION_H_
#define DATAINFORMATION_H_

//Qt core
#include <QString>
#include <QList>

//Okteta
#include <size.h>
#include <address.h>

#include "structviewpreferences.h"
#include "additionaldata.h"

//KDE
#include <KLocale>
#include <KGlobal>

//Qt
#include <QtScript/QScriptValue>


#include "topleveldatainformation.h"
#include "datainformationbase.h"

#define DATAINFORMATION_CLONE(type) virtual inline DataInformation* clone() const {\
        return new type##DataInformation(*this); \
    }

namespace Okteta
{
class AbstractByteArrayModel;
}

class TopLevelDataInformation;
class ScriptHandlerInfo;
class QScriptContext;
class QScriptEngine;

typedef Kasten::StructViewPreferences::EnumByteOrder::type ByteOrder;
typedef Kasten::StructViewPreferences::EnumByteOrder ByteOrderEnumClass;

/** Interface that must be implemented by all datatypes */
class DataInformation: public DataInformationBase
{
protected:
    DataInformation(const DataInformation&);
public:
    virtual DataInformation* clone() const = 0;
    explicit DataInformation(const QString& name, DataInformationBase* parent = NULL);
    virtual ~DataInformation();

    enum Columns
    {
        ColumnName = 0, ColumnType, ColumnValue, COLUMN_COUNT
    };
    enum DataInformationEndianess {
        EndianessFromSettings = 0, EndianessInherit, EndianessLittle, EndiannessBig
    };

    //methods for children:
    virtual bool hasChildren() const;
    /** true for unions and structs and arrays*/
    virtual bool canHaveChildren() const;
    virtual unsigned int childCount() const;
    virtual DataInformation* childAt(unsigned int) const;
    virtual quint64 positionRelativeToParent(int row = -1) const;

    //for the model:
    virtual Qt::ItemFlags flags(int column, bool fileLoaded = true) const;
    int row() const;
    /** get the necessary data (for the model) */
    virtual QVariant data(int column, int role) const;
    /** the data of child at index @p row. Useful for arrays, or DataInformations with fake children*/
    virtual QVariant childData(int row, int column, int role) const;
    virtual Qt::ItemFlags childFlags(int row, int column, bool fileLoaded = true) const;
    virtual int childSize(int index) const;


    /** The size of this DataInformation type in bits (to allow bitfields in future) */
    virtual QString typeName() const = 0;
    /** by default just returns an empty QString */
    virtual QString valueString() const;
    virtual QString name() const;
    virtual QString tooltipString() const;
    /** needs to be virtual for bitfields */
    virtual QString sizeString() const;

    //delegate functions:
    /** create a QWidget for the QItemDelegate */
    virtual QWidget* createEditWidget(QWidget* parent) const = 0;
    /** get the needed data from the widget */
    virtual QVariant dataFromWidget(const QWidget* w) const = 0;
    /** initialize the delegate widget with the correct data */
    virtual void setWidgetData(QWidget* w) const = 0;

    //reading and writing
    /** the size in bits of this element */
    virtual int size() const = 0;
    
    /** Reads the necessary data from @p input and returns the number of bytes read
     *
     * @param input the byte array to read from
     * @param address the starting offset to read from
     * @param bitsRemaining the number of bits remaining in @p out
     * @param bitOffset the bits that have already been read from the current byte
     *        (should be modified in this method)
     *
     * @return the number of bits read or @c -1 if none were read
     */
    virtual qint64 readData(Okteta::AbstractByteArrayModel *input, Okteta::Address address,
             quint64 bitsRemaining, quint8* bitOffset) = 0;
    /** sets mWasAbleToRead to false for all children and this object.
     *  Gets called once before the reading of the whole structure starts. */
    void beginRead();
    /** Writes the current data contained in this object to out.
     *
     *  If @code @p inf != this @endcode this method must do nothing and return false.
     *
     *  @param value a @link QVariant object holding the new data to write
     *  @param inf the object that should currently write the data
     *  @param out the byte array the value is read from
     *  @param address the address in @p out
     *  @param bitsRemaining number of bits remaining in @p input
     *  @param bitOffset the bit to start at in the first byte
     *
     *  @return @c true on success, @c false otherwise
     */
    virtual bool setData(const QVariant& value, DataInformation* inf,
            Okteta::AbstractByteArrayModel *input, Okteta::Address address,
            quint64 bitsRemaining, quint8* bitOffset) = 0;

    virtual bool setChildData(int row, const QVariant& value, DataInformation* inf,
            Okteta::AbstractByteArrayModel *input, Okteta::Address address,
            quint64 bitsRemaining, quint8* bitOffset);
    virtual bool isTopLevel() const;
    TopLevelDataInformation* topLevelDataInformation() const;
    DataInformation* mainStructure();

    void setAdditionalData(AdditionalData* data);
    AdditionalData* additionalData() const;
    QString validationError() const;
    void setValidationError(QString errorMessage);

    bool validationSuccessful() const;
    void setValidationSuccessful(bool validationSuccessful = true);
    bool hasBeenValidated() const;
    void setHasBeenValidated(bool hasBeen);
    ByteOrder byteOrder() const;
    void setByteOrder(DataInformationEndianess newEndianess);
    void setByteOrder(ByteOrder newByteOrder);

    virtual void resetValidationState(); //virtual for datainformationwithchildren
    bool wasAbleToRead() const;
    virtual QScriptValue toScriptValue(QScriptEngine* engine, ScriptHandlerInfo* handlerInfo) = 0;
    void setParent(DataInformationBase* newParent);
    DataInformationBase* parent() const;
protected:
    /**
     *  the offset of child number @p index compared to the beginning of the structure
     *  in bits.
     *  @param index the index of the child
     *  @return 0 unless this DataInformation has children
     */
    virtual quint64 offset(unsigned int index) const = 0;
    /**
     * Find the index of a DataInformation in this object, needed to calculate the row
     */
    virtual int indexOf(const DataInformation* const data) const;
protected:
    bool mValidationSuccessful :1;
    bool mHasBeenValidated :1;
    bool mWasAbleToRead :1;
    DataInformationEndianess mByteOrder : 2;
    AdditionalData* mAdditionalData;
    DataInformationBase* mParent;
    QString mName;
};

Q_DECLARE_METATYPE(DataInformation*)
Q_DECLARE_METATYPE(const DataInformation*)

//inline functions
inline int DataInformation::row() const
{
    Q_CHECK_PTR(mParent);
    if (mParent->isTopLevel())
        return static_cast<TopLevelDataInformation*>(mParent)->indexOf(this);
    else 
        return static_cast<const DataInformation*>(mParent)->indexOf(this);
}

inline Qt::ItemFlags DataInformation::flags(int column, bool fileLoaded) const
{
    Q_UNUSED(column)
    Q_UNUSED(fileLoaded);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

inline QString DataInformation::name() const
{
    return mName;
}

inline DataInformation* DataInformation::childAt(unsigned int) const
{
    return NULL;
}

inline bool DataInformation::hasChildren() const
{
    return false;
}

inline bool DataInformation::canHaveChildren() const
{
    return false;
}

inline unsigned int DataInformation::childCount() const
{
    return 0;
}

inline AdditionalData* DataInformation::additionalData() const
{
    return mAdditionalData;
}

inline bool DataInformation::wasAbleToRead() const
{
    return mWasAbleToRead;
}

inline ByteOrder DataInformation::byteOrder() const
{
    switch (mByteOrder)
    {
        case EndiannessBig: 
            return ByteOrderEnumClass::BigEndian;
        case EndianessLittle: 
            return ByteOrderEnumClass::LittleEndian;
        case EndianessFromSettings:
            return Kasten::StructViewPreferences::byteOrder();
        case EndianessInherit:
            return (mParent && !mParent->isTopLevel()) ?
                static_cast<DataInformation*>(mParent)->byteOrder()
                : Kasten::StructViewPreferences::byteOrder();
    }
    
    // here must be a return... I guess this is correct
    return Kasten::StructViewPreferences::byteOrder();
}

inline void DataInformation::setByteOrder(DataInformation::DataInformationEndianess newByteOrder)
{
    mByteOrder = newByteOrder;
}

inline void DataInformation::setByteOrder(ByteOrder newByteOrder)
{
    //XXX is this method needed?
    mByteOrder = newByteOrder == ByteOrderEnumClass::BigEndian ? EndiannessBig : EndianessLittle;
}

inline bool DataInformation::isTopLevel() const
{
    return false;
}

inline void DataInformation::setParent(DataInformationBase* newParent)
{
    Q_CHECK_PTR(newParent);
    mParent = newParent;
}

inline DataInformationBase* DataInformation::parent() const
{
    return mParent;
}

inline TopLevelDataInformation* DataInformation::topLevelDataInformation() const
{
    Q_CHECK_PTR(mParent);
    if (mParent->isTopLevel())
        return static_cast<TopLevelDataInformation*>(mParent);

    return static_cast<const DataInformation*>(mParent)->topLevelDataInformation();
}

#endif /* DATAINFORMATION_H_ */
