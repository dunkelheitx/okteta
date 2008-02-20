/***************************************************************************
                          replacepiecetablechange.cpp  -  description
                             -------------------
    begin                : Mon Feb 11 2008
    copyright            : 2008 by Friedrich W. H. Kossebau
    email                : kossebau@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/

#include "replacepiecetablechange.h"

// lib
#include "piecetable.h"
#include <ksection.h>
#include <kreplacementscope.h>
// KDE
#include <KLocale>


namespace KPieceTable
{

int ReplacePieceTableChange::type() const { return ReplaceId; }

QString ReplacePieceTableChange::description() const
{
    return i18n( "Replace" );
}

bool ReplacePieceTableChange::merge( const AbstractPieceTableChange *other )
{
    bool result = false;
    if( other->type() == ReplaceId )
    {
        const ReplacePieceTableChange *otherReplaceChange = static_cast<const ReplacePieceTableChange *>( other );
        // other replaced after?
        if( mRemoveSection.start()+mInsertLength == otherReplaceChange->mRemoveSection.start() )
        {
            mRemoveSection.moveEndBy( otherReplaceChange->mRemoveSection.width() );
            mInsertLength += otherReplaceChange->mInsertLength;
            mRemovedPieces.append( otherReplaceChange->mRemovedPieces );
            result = true;
        }
        // other replaced before would be two swapped ranges in the change buffer, if this ever needed/wanted?
    }

    return result;
}

KHE::KSection ReplacePieceTableChange::apply( PieceTable *pieceTable )
{
    const int oldSize = pieceTable->size();

    pieceTable->replace( mRemoveSection, mInsertLength, mStorageOffset );

    const int newSize = pieceTable->size();
    const int lastChanged = ( newSize == oldSize ) ? mRemoveSection.end() :
                            ( newSize > oldSize ) ?  newSize - 1 :
                                                     oldSize - 1;
    return KHE::KSection( mRemoveSection.start(), lastChanged );
}

KHE::KSection ReplacePieceTableChange::revert( PieceTable *pieceTable )
{
    const int oldSize = pieceTable->size();

    const KHE::KSection insertedSection = KHE::KSection::fromWidth( mRemoveSection.start(), mInsertLength );
    pieceTable->replace( insertedSection, mRemovedPieces );

    const int newSize = pieceTable->size();
    const int lastChanged = ( newSize == oldSize ) ? insertedSection.end() :
                            ( newSize > oldSize ) ?  newSize - 1 :
                                                     oldSize - 1;
    return KHE::KSection( mRemoveSection.start(), lastChanged );
}

KHE::ReplacementScope ReplacePieceTableChange::replacement() const
{ return KHE::ReplacementScope(mRemoveSection.start(),mRemoveSection.width(),mInsertLength); }

ReplacePieceTableChange::~ReplacePieceTableChange() {}

}
