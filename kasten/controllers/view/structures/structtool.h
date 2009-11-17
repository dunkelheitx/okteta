/*
 *   This file is part of the Okteta Kasten module, part of the KDE project.
 *
 *   Copyright 2009 Alex Richardson <alex.richardson@gmx.de>
 *   Copyright 2009 Friedrich W. H. Kossebau <kossebau@kde.org>
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
#ifndef STRUCTTOOL_H_
#define STRUCTTOOL_H_

#include <KDebug>
// lib
#include "oktetakastencontrollers_export.h"
// tool
#include "structviewpreferences.h"
#include "datatypes/datainformation.h"
#include "datatypes/datainformationgroup.h"
// Kasten core
#include <abstracttool.h>
// Okteta core
#include <address.h>
//Qt
#include <QModelIndex>
#include <QFileInfo>
#include <QStringList>

namespace Okteta
{
class CharCodec;
class AbstractByteArrayModel;
}

namespace Kasten
{
class ByteArrayView;

class OKTETAKASTENCONTROLLERS_EXPORT StructTool: public AbstractTool
{
Q_OBJECT
public:
    StructTool();
    virtual ~StructTool();
public:
    // AbstractTool API
    //     virtual AbstractModel* targetModel() const;
    virtual QString title() const;
    virtual void setTargetModel(AbstractModel* model);

public:
    StructViewPreferences::EnumByteOrder::type byteOrder() const
    {
        return mByteOrder;
    }
    void setByteOrder(StructViewPreferences::EnumByteOrder::type order);
    inline int columnCount()
    {
        return DataInformation::COLUMN_COUNT;
    }
    bool setData(const QVariant& value, int role, DataInformation* item);
    Qt::ItemFlags flags(int column, DataInformation* data) const
    {
        return data->flags(column, mByteArrayModel != NULL);
    }
public Q_SLOTS:
    void setByteOrder(int order);

Q_SIGNALS: // changes to the setting currently not signalled, because only controlled by view
    void dataChanged();
    void dataCleared();
    void byteOrderChanged();

public Q_SLOTS:
    void mark(const QModelIndex& idx);
    void unmark(/*const QModelIndex& idx*/);
    void updateData();
    void addChildItem(DataInformation* child);
    void loadStructDefFiles();

protected Q_SLOTS:
    void onCursorPositionChange(Okteta::Address pos);
    void onContentsChange();
    //	void onCharCodecChange(const QString& codecName);
    void onChildItemDataChanged()
    {
        //		kDebug() << "StructTool::onChildItemDataChanged()";
        emit dataChanged();
    }

protected:
    // source
    ByteArrayView* mByteArrayView;
    Okteta::AbstractByteArrayModel* mByteArrayModel;
    Okteta::Address mCursorIndex;
    //	Okteta::CharCodec* mCharCodec;

protected:
    // settings
    StructViewPreferences::EnumByteOrder::type mByteOrder;
    QStringList mLoadedFiles;
    QList<DataInformationGroup*> mLoadedDefs;
    QList<DataInformation*> mData;

    DataInformationGroup* loadXML(QFileInfo& file, QStringList& includedFiles);
    void parseIncludeNode(const QDomElement& elem, const QFileInfo& fileInfo,
            QStringList& includedFiles);
    // decoded data

    //interface for model
public:
    QVariant headerData(int column, int role);
    int childCount() const;
    DataInformation* childAt(int idx) const;
    //	QTextCodec* mUtf8Codec; //XXX add utf8 strings sometime
};
}
#endif /* STRUCTTOOL_H_ */