/*
    This file is part of the Okteta Kasten module, made within the KDE community.

    Copyright 2007-2008 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef BYTETABLETOOL_HPP
#define BYTETABLETOOL_HPP

// lib
#include <kasten/okteta/oktetakastencontrollers_export.hpp>
// Kasten core
#include <Kasten/AbstractTool>

namespace Okteta {
class AbstractByteArrayModel;
}

namespace Kasten {

class ByteTableModel;
class ByteArrayView;

/**
 */
class OKTETAKASTENCONTROLLERS_EXPORT ByteTableTool : public AbstractTool
{
    Q_OBJECT

public:
    ByteTableTool();
    ~ByteTableTool() override;

public: // AbstractTool API
//     virtual AbstractModel* targetModel() const;
    QString title() const override;

    void setTargetModel(AbstractModel* model) override;

public:
    void insert(unsigned char byte, int count);

public:
    ByteTableModel* byteTableModel() const;
    bool hasWriteable() const;

Q_SIGNALS:
    void hasWriteableChanged(bool hasWriteable);

private Q_SLOTS:
    void onReadOnlyChanged(bool isReadOnly);

private:
    ByteTableModel* mByteTableModel;

    ByteArrayView* mByteArrayView = nullptr;
    Okteta::AbstractByteArrayModel* mByteArrayModel = nullptr;
};

}

#endif