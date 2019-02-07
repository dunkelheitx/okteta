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

#ifndef KASTEN_BYTEARRAYVIEWTEXTSTREAMENCODER_HPP
#define KASTEN_BYTEARRAYVIEWTEXTSTREAMENCODER_HPP

// lib
#include "abstractbytearraystreamencoder.hpp"
// Okteta gui
// #include <Okteta/OffsetFormat>
// Okteta core
#include <Okteta/OktetaCore>
// Qt
#include <QString>

namespace Kasten {

class ByteArrayViewTextStreamEncoderSettings
{
public:
    ByteArrayViewTextStreamEncoderSettings();

public:
//     Okteta::OffsetFormat::Format offsetFormat;
    int codingWidth;
    int firstLineOffset;
    int startOffset;
    int delta;
    Okteta::ValueCoding valueCoding = Okteta::HexadecimalCoding;
    QString codecName;
    QChar undefinedChar = {QLatin1Char('?')};
    QChar substituteChar = {QLatin1Char('.')};
    QString separation;
//     Okteta::CharCodec *CharCodec;
//     Okteta::OffsetFormat::print printFunction;
};

// TODO: this could rather be one of the default cop
class ByteArrayViewTextStreamEncoder : public AbstractByteArrayStreamEncoder
{
    Q_OBJECT

public:
    ByteArrayViewTextStreamEncoder();
    ~ByteArrayViewTextStreamEncoder() override;

protected: // AbstractByteArrayStreamEncoder API
    bool encodeDataToStream(QIODevice* device,
                            const ByteArrayView* byteArrayView,
                            const Okteta::AbstractByteArrayModel* byteArrayModel,
                            const Okteta::AddressRange& range) override;

protected:
    ByteArrayViewTextStreamEncoderSettings mSettings;
};

}

#endif
