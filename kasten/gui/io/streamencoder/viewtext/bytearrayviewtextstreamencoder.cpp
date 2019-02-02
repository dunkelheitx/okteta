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

#include "bytearrayviewtextstreamencoder.hpp"

// lib
#include "offsetcolumntextrenderer.hpp"
#include "bordercolumntextrenderer.hpp"
#include "valuebytearraycolumntextrenderer.hpp"
#include "charbytearraycolumntextrenderer.hpp"
#include "bytearrayrowscolumntextrenderer.hpp"
#include <bytearrayview.hpp>
// Okteta gui
#include <Okteta/ByteArrayTableLayout>
#include <Okteta/AbstractByteArrayView>
// Okteta core
#include <Okteta/AbstractByteArrayModel>
#include <Okteta/Character>
// KF5
#include <KLocalizedString>
// Qt
#include <QTextStream>

namespace Kasten {

// static const Okteta::OffsetFormat::Format DefaultOffsetFormat = Okteta::OffsetFormat::Hexadecimal;

ByteArrayViewTextStreamEncoderSettings::ByteArrayViewTextStreamEncoderSettings()
    // : offsetFormat(DefaultOffsetFormat)
    : separation(QStringLiteral(" "))
{}

ByteArrayViewTextStreamEncoder::ByteArrayViewTextStreamEncoder()
    : AbstractByteArrayStreamEncoder(i18nc("name of the encoding target", "View in Plain Text"), QStringLiteral("text/plain"))
{}

ByteArrayViewTextStreamEncoder::~ByteArrayViewTextStreamEncoder() = default;

bool ByteArrayViewTextStreamEncoder::encodeDataToStream(QIODevice* device,
                                                        const ByteArrayView* byteArrayView,
                                                        const Okteta::AbstractByteArrayModel* byteArrayModel,
                                                        const Okteta::AddressRange& range)
{
    bool success = true;

    // settings
    mSettings.codecName = byteArrayView->charCodingName();
    mSettings.valueCoding = (Okteta::ValueCoding)byteArrayView->valueCoding();
    mSettings.undefinedChar = byteArrayView->undefinedChar();
    mSettings.substituteChar = byteArrayView->substituteChar();
    mSettings.firstLineOffset = byteArrayView->firstLineOffset();
    mSettings.startOffset = byteArrayView->startOffset();
    mSettings.delta = byteArrayView->noOfBytesPerLine();
    const int viewModus = byteArrayView->viewModus();

    // setup
    Okteta::ByteArrayTableLayout layout(byteArrayView->noOfBytesPerLine(), mSettings.firstLineOffset,
                                        mSettings.startOffset, 0, byteArrayModel->size());

    Okteta::CoordRange coordRange;
    coordRange.set(layout.coordRangeOfIndizes(range));

    const int noOfBytesPerLine = byteArrayView->noOfBytesPerLine();
    const int byteSpacingWidth = byteArrayView->byteSpacingWidth();
    const int noOfGroupedBytes = byteArrayView->noOfGroupedBytes();
    const int visibleByteArrayCodings = byteArrayView->visibleByteArrayCodings();

    QList<AbstractColumnTextRenderer*> columnTextRendererList;

    if (byteArrayView->offsetColumnVisible()) {
        columnTextRendererList.append(
            new OffsetColumnTextRenderer(Okteta::OffsetFormat::Hexadecimal, mSettings.firstLineOffset, mSettings.delta));
        columnTextRendererList.append(new BorderColumnTextRenderer());
    }

    if (viewModus == 0) {
        if (visibleByteArrayCodings & Okteta::AbstractByteArrayView::ValueCodingId) {
            columnTextRendererList.append(
                new ValueByteArrayColumnTextRenderer(byteArrayModel, range.start(), coordRange,
                                                     noOfBytesPerLine, byteSpacingWidth, noOfGroupedBytes,
                                                     mSettings.valueCoding));
        }

        if (visibleByteArrayCodings & Okteta::AbstractByteArrayView::CharCodingId) {
            if (visibleByteArrayCodings & Okteta::AbstractByteArrayView::ValueCodingId) {
                columnTextRendererList.append(new BorderColumnTextRenderer());
            }
            columnTextRendererList.append(
                new CharByteArrayColumnTextRenderer(byteArrayModel, range.start(), coordRange,
                                                    noOfBytesPerLine, 0, 0,
                                                    mSettings.codecName, mSettings.substituteChar, mSettings.undefinedChar));
        }
    } else {
        columnTextRendererList.append(
            new ByteArrayRowsColumnTextRenderer(byteArrayModel, range.start(), coordRange,
                                                noOfBytesPerLine, byteSpacingWidth, noOfGroupedBytes,
                                                visibleByteArrayCodings,
                                                mSettings.valueCoding,
                                                mSettings.codecName, mSettings.substituteChar, mSettings.undefinedChar));
    }

    int subLinesCount = 1;
    for (const AbstractColumnTextRenderer* renderer : qAsConst(columnTextRendererList)) {
        if (renderer->noOfSublinesNeeded() > subLinesCount) {
            subLinesCount = renderer->noOfSublinesNeeded();
        }
    }

    // encode
    QTextStream textStream(device);

    int l = coordRange.start().line();
    for (const AbstractColumnTextRenderer* renderer : qAsConst(columnTextRendererList)) {
        renderer->renderFirstLine(&textStream, l);
    }

    textStream << endl;

    int subLine = 1;
    while (true) {
        if (subLine == subLinesCount) {
            ++l;
            if (l > coordRange.end().line()) {
                break;
            }
            subLine = 0;
        }
        const bool isSubline = (subLine > 0);
        for (const AbstractColumnTextRenderer* renderer : qAsConst(columnTextRendererList)) {
            renderer->renderNextLine(&textStream, isSubline);
        }

        textStream << endl;
        ++subLine;
    }

    // clean up
    qDeleteAll(columnTextRendererList);

    return success;
}

}
