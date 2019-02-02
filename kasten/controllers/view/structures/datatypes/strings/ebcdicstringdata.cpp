/*
 *   This file is part of the Okteta Kasten Framework, made within the KDE community.
 *
 *   Copyright 2011 Alex Richardson <alex.richardson@gmx.de>
 *   Copyright 2016 Aaron Bishop <erroneous@gmail.com>
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

#include "ebcdicstringdata.hpp"
#include "stringdatainformation.hpp"
#include "../topleveldatainformation.hpp"
#include "../../structlogging.hpp"

#include <Okteta/Character>
#include <codecs/ebcdic1047charcodec.hpp>
#include <Okteta/AbstractByteArrayModel>

#include <KLocalizedString>
#include <QVarLengthArray>

EbcdicStringData::EbcdicStringData(StringDataInformation* parent)
    : StringData(parent)
    , mCodec(Okteta::EBCDIC1047CharCodec::create())
{
}

EbcdicStringData::~EbcdicStringData()
{
    delete mCodec;
}

qint64 EbcdicStringData::read(Okteta::AbstractByteArrayModel* input, Okteta::Address address, BitCount64 bitsRemaining)
{
    const int oldSize = count();
    if (mMode == CharCount || mMode == ByteCount) { // same for ebcdic
        mData.reserve(mLength.maxChars);
    }
    mParent->topLevelDataInformation()->_childCountAboutToChange(mParent, oldSize, 0);
    mParent->topLevelDataInformation()->_childCountChanged(mParent, oldSize, 0);

    quint64 remaining = bitsRemaining;
    Okteta::Address addr = address;
    int count = 0;
    mEofReached = false;
    const int oldMax = mData.size();
    if (((mMode & CharCount) && mLength.maxChars == 0) || ((mMode & ByteCount) && mLength.maxBytes == 0)) {
        return 0; // nothing to read

    }
    bool eofAtStart = false;
    if (bitsRemaining < 8) {
        eofAtStart = true;
    }

    while (true) {
        if (remaining < 8) {
            mEofReached = true;
            break;
        }
        uchar val = input->byte(addr);
        bool terminate = false;

        if (count < oldMax) {
            mData[count] = val;
        } else {
            mData.append(val);
        }

        remaining -= 8;
        addr++;
        count++;

        // now check if we have to terminate
        if (mMode & Sequence) {
            if ((quint32(val) & 0xff) == mTerminationCodePoint) {
                terminate = true;
            }
        }
        if ((mMode & CharCount)  || (mMode & ByteCount)) {
            if ((unsigned)count >= mLength.maxChars) {
                terminate = true;
            }
        }
        if (mMode == None) {
            qCDebug(LOG_KASTEN_OKTETA_CONTROLLERS_STRUCTURES) << "no termination mode set!!";
            Q_ASSERT(false);
        }
        if (terminate) {
            break;
        }
    }

    mData.resize(count);
    mParent->topLevelDataInformation()->_childCountAboutToChange(mParent, 0, count);
    mParent->topLevelDataInformation()->_childCountChanged(mParent, 0, count);

    if (eofAtStart) {
        return -1;
    }
    return (addr - address) * 8;
}

BitCount32 EbcdicStringData::sizeAt(uint i) const
{
    Q_ASSERT(i < count());
    Q_UNUSED(i)
    return 8;
}

BitCount32 EbcdicStringData::size() const
{
    return mData.size() * 8;
}

QString EbcdicStringData::completeString(bool) const
{
    int max = mData.size();
    QVarLengthArray<QChar> buf(max);
    for (int i = 0; i < max; ++i) {
        uchar val = mData.at(i);
        buf[i] = mCodec->decode(val);
    }

    return QString(buf.constData(), max);
}

QString EbcdicStringData::stringValue(int row) const
{
    Q_ASSERT(row >= 0 && row < mData.size());
    uchar val = mData.at(row);
    return mCodec->decode(val);
}

QString EbcdicStringData::charType() const
{
    return i18n("EBCDIC char");
}

uint EbcdicStringData::count() const
{
    return mData.size();
}

QString EbcdicStringData::typeName() const
{
    return i18n("EBCDIC string");
}
