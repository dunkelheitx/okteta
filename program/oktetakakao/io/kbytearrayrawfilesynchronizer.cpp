/*
    This file is part of the Okteta Kakao module, part of the KDE project.

    Copyright 2007 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "kbytearrayrawfilesynchronizer.h"

// lib
#include "kbytearraydocument.h"
// Okteta core
#include <kpiecetablebytearraymodel.h>
// KDE
#include <KUrl>
#include <KLocale>
// Qt
#include <QtCore/QDataStream>
#include <QtCore/QFile>



KByteArrayRawFileSynchronizer::KByteArrayRawFileSynchronizer( const KUrl &url )
{
    KAbstractDocument *document = loadFromUrl( url );
    mDocument = document ? qobject_cast<KByteArrayDocument*>( document ) : 0;
    if( mDocument )
        onUrlChange( url );
    connect( this, SIGNAL(urlChanged(const KUrl&)), SLOT(onUrlChange( const KUrl & )) );
}

KByteArrayRawFileSynchronizer::KByteArrayRawFileSynchronizer( KAbstractDocument *document, const KUrl &url,
                                                              KAbstractDocumentSynchronizer::ConnectOption option )
{
    // TODO: is synchronizer->document() really a good signal for success? see also above
    mDocument = document ? qobject_cast<KByteArrayDocument*>( document ) : 0;
    if( mDocument )
    {
        if( !syncWithUrl(url,option) )
            mDocument = 0;
    }
    if( mDocument )
        onUrlChange( url );
    connect( this, SIGNAL(urlChanged(const KUrl&)), SLOT(onUrlChange( const KUrl & )) );
}

KAbstractDocument *KByteArrayRawFileSynchronizer::document() const { return mDocument; }

KAbstractDocument *KByteArrayRawFileSynchronizer::loadFromFile( const QString &workFilePath/*, int *success*/ )
{
    KByteArrayDocument *document = 0;

    QFile file( workFilePath );
    file.open( QIODevice::ReadOnly );
    QDataStream inStream( &file );
    int fileSize = file.size();

    // TODO: should the decoder know this?
    // it is in the api now (constructor)

    char *data = new char[fileSize];
    inStream.readRawData( data, fileSize );
    KHECore::KPieceTableByteArrayModel *byteArray = new KHECore::KPieceTableByteArrayModel( data, fileSize );

    byteArray->setModified( false );

    //registerDiskModifyTime( file ); TODO move into synchronizer

    const bool streamIsOk = ( inStream.status() == QDataStream::Ok );
//     if( success )
//         *success = streamIsOk ? 0 : 1;
    if( streamIsOk )
        document = new KByteArrayDocument( byteArray, i18nc("destination of the byte array", "Loaded from file.") );
    else
        delete byteArray;

    return document;
}

bool KByteArrayRawFileSynchronizer::reloadFromFile( const QString &workFilePath )
{
    QFile file( workFilePath );
    file.open( QIODevice::ReadOnly );
    QDataStream inStream( &file );
    int fileSize = file.size();

    // TODO: should the decoder know this?
    char *newData = new char[fileSize];
    inStream.readRawData( newData, fileSize );

    //registerDiskModifyTime( file ); TODO move into synchronizer

    const bool streamIsOk = ( inStream.status() == QDataStream::Ok );
//     if( success )
//         *success = streamIsOk ? 0 : 1;
    if( streamIsOk )
    {
        KHECore::KPieceTableByteArrayModel *byteArray = qobject_cast<KHECore::KPieceTableByteArrayModel*>( mDocument->content() );
        byteArray->setData( newData, fileSize, false );
    }
    else
        delete [] newData;

    return streamIsOk;
}

bool KByteArrayRawFileSynchronizer::writeToFile( const QString &workFilePath )
{
    KHECore::KPieceTableByteArrayModel *byteArray = qobject_cast<KHECore::KPieceTableByteArrayModel*>( mDocument->content() );

    QFile file( workFilePath );
    file.open( QIODevice::WriteOnly );

    QDataStream outStream( &file );

    //TODO: this was
//     outStream.writeRawData( byteArray->data(), byteArray->size() );
    // make it quicker again by writing spans -> spaniterator

    for( int i = 0; i<byteArray->size(); ++i )
    {
        const char datum = byteArray->datum(i);
        outStream.writeRawData( &datum, 1 );
    }

    byteArray->setModified( false );

    //registerDiskModifyTime( file );TODO move into synchronizer

    return outStream.status() == QDataStream::Ok;
}

bool KByteArrayRawFileSynchronizer::syncWithFile( const QString &workFilePath,
                                                  KAbstractDocumentSynchronizer::ConnectOption option )
{
Q_UNUSED( option );
    return writeToFile( workFilePath );
}

void KByteArrayRawFileSynchronizer::onUrlChange( const KUrl &url )
{
    mDocument->setTitle( url.fileName() );
}
