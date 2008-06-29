/*
    This file is part of the Okteta Kakao module, part of the KDE project.

    Copyright 2008 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "abstractfilesystemsynctoremotejob.h"

// library
#include "kabstractdocumentfilesystemsynchronizer.h"
#include <kabstractdocument.h>
// KDE
#include <KIO/NetAccess>
#include <KTemporaryFile>
#include <KLocale>
#include <KDirWatch>
// Qt
#include <QtCore/QTimer>


class AbstractFileSystemSyncToRemoteJob::Private
{
  public:
    Private( KAbstractDocumentFileSystemSynchronizer *synchronizer );

  public:
    void setWorkFilePath( const QString &workFilePath );

  public:
    KUrl url() const;
    QString workFilePath() const;
    QWidget *widget() const;
    KAbstractDocumentFileSystemSynchronizer *synchronizer() const;

  protected:
    KAbstractDocumentFileSystemSynchronizer *mSynchronizer;
    QString mWorkFilePath;
};

AbstractFileSystemSyncToRemoteJob::Private::Private( KAbstractDocumentFileSystemSynchronizer *synchronizer )
 : mSynchronizer( synchronizer )
{}

inline KUrl AbstractFileSystemSyncToRemoteJob::Private::url()             const { return mSynchronizer->url(); }
inline QString AbstractFileSystemSyncToRemoteJob::Private::workFilePath() const { return mWorkFilePath; }
// TODO: setup a notification system
inline QWidget *AbstractFileSystemSyncToRemoteJob::Private::widget()      const { return 0; }
inline KAbstractDocumentFileSystemSynchronizer *AbstractFileSystemSyncToRemoteJob::Private::synchronizer() const
{
    return mSynchronizer;
}

inline void AbstractFileSystemSyncToRemoteJob::Private::setWorkFilePath( const QString &workFilePath )
{
    mWorkFilePath = workFilePath;
}



AbstractFileSystemSyncToRemoteJob::AbstractFileSystemSyncToRemoteJob( KAbstractDocumentFileSystemSynchronizer *synchronizer )
 : d( new Private(synchronizer) )
{}

KAbstractDocumentFileSystemSynchronizer *AbstractFileSystemSyncToRemoteJob::synchronizer() const
{
    return d->synchronizer();
}
QString AbstractFileSystemSyncToRemoteJob::workFilePath() const { return d->workFilePath(); }
QWidget *AbstractFileSystemSyncToRemoteJob::widget() const { return d->widget(); }

void AbstractFileSystemSyncToRemoteJob::start()
{
    QTimer::singleShot( 0, this, SLOT(syncToRemote()) );
}

void AbstractFileSystemSyncToRemoteJob::syncToRemote()
{
    KTemporaryFile temporaryFile;

    if( d->url().isLocalFile() )
        d->setWorkFilePath( d->url().path() );
    else
    {
        temporaryFile.open();
        d->setWorkFilePath( temporaryFile.fileName() );
    }
    startWriteToFile();
}

void AbstractFileSystemSyncToRemoteJob::completeWrite( bool success )
{
    if( success )
    {
        if( !d->url().isLocalFile() )
        {
            success = KIO::NetAccess::upload( d->workFilePath(), d->url(), d->widget() );
            if( !success )
            {
                setError( KilledJobError );
                setErrorText( KIO::NetAccess::lastErrorString() );
            }
        }
    }
    else
    {
        setError( KilledJobError );
        setErrorText( i18nc("@info","Problem when saving to local filesystem.") );
    }

    emitResult();
}


AbstractFileSystemSyncToRemoteJob::~AbstractFileSystemSyncToRemoteJob()
{
    delete d;
}
