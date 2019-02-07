/*
    This file is part of the Okteta Kasten module, made within the KDE community.

    Copyright 2011 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KASTEN_CHARSETCONVERSIONJOB_HPP
#define KASTEN_CHARSETCONVERSIONJOB_HPP

// Okteta core
#include <Okteta/AddressRange>
#include <Okteta/Byte>
// Qt
#include <QMap>
#include <QObject>

namespace Okteta {
class AbstractByteArrayModel;
class CharCodec;
}

namespace Kasten {

class CharsetConversionJob : public QObject // not yet: KJob
{
    Q_OBJECT

public:
    CharsetConversionJob(Okteta::Byte* result,
                         Okteta::AbstractByteArrayModel* model,
                         const Okteta::AddressRange& range,
                         const Okteta::CharCodec* sourceCharCodec,
                         const Okteta::CharCodec* targetCharCodec,
                         bool isSubstitutingMissingChars,
                         Okteta::Byte substituteByte);

public:
    bool exec();

public:
    int convertedBytesCount() const;
    const QMap<Okteta::Byte, int>& failedPerByteCount() const;

protected:
    Okteta::Byte* const mResult;
    Okteta::AbstractByteArrayModel* const mByteArrayModel;
    const Okteta::AddressRange mRange;

    const Okteta::CharCodec* const mSourceCharCodec;
    const Okteta::CharCodec* const mTargetCharCodec;

    bool mSubstitutingMissingChars;
    Okteta::Byte mSubstituteByte;

    int mConvertedBytesCount;
    QMap<Okteta::Byte, int> mFailedPerByteCount;
};

inline CharsetConversionJob::CharsetConversionJob(Okteta::Byte* result,
                                                  Okteta::AbstractByteArrayModel* model,
                                                  const Okteta::AddressRange& range,
                                                  const Okteta::CharCodec* sourceCharCodec,
                                                  const Okteta::CharCodec* targetCharCodec,
                                                  bool isSubstitutingMissingChars,
                                                  Okteta::Byte substituteByte)
    : mResult(result)
    , mByteArrayModel(model)
    , mRange(range)
    , mSourceCharCodec(sourceCharCodec)
    , mTargetCharCodec(targetCharCodec)
    , mSubstitutingMissingChars(isSubstitutingMissingChars)
    , mSubstituteByte(substituteByte)
{}

inline int CharsetConversionJob::convertedBytesCount() const { return mConvertedBytesCount; }

inline const QMap<Okteta::Byte, int>& CharsetConversionJob::failedPerByteCount() const { return mFailedPerByteCount; }

}

#endif
