/*
    This file is part of the Kakao Framework, part of the KDE project.

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

#include "versioncontroller.h"

// Kakao gui
#include <kabstractview.h>
// Kakao core
#include <kdocumentversiondata.h>
#include <kiversionable.h>
// KDE
#include <KXmlGuiWindow>
#include <KLocale>
#include <KActionCollection>
#include <KToolBarPopupAction>
#include <KStandardShortcut>
// Qt
#include <QtGui/QMenu>


static const int MaxMenuEntries = 10;

VersionController::VersionController( KXmlGuiWindow *window )
 : mDocument( 0 )
{
    KActionCollection *actionCollection = window->actionCollection();

    mSetToOlderVersionAction = new KToolBarPopupAction( KIcon("edit-undo"), i18n("Undo"), this );
    actionCollection->addAction( "edit_undo", mSetToOlderVersionAction );
    mSetToOlderVersionAction->setShortcuts( KStandardShortcut::undo() );

    connect( mSetToOlderVersionAction, SIGNAL(triggered( bool )),
             SLOT(onSetToOlderVersionTriggered()) );
    connect( mSetToOlderVersionAction->menu(), SIGNAL(aboutToShow()),
             SLOT(onOlderVersionMenuAboutToShow()) );
    connect( mSetToOlderVersionAction->menu(), SIGNAL(triggered( QAction* )),
             SLOT(onOlderVersionMenuTriggered( QAction* )) );

    mSetToNewerVersionAction = new KToolBarPopupAction( KIcon("edit-redo"), i18n("Redo"), this );
    actionCollection->addAction( "edit_redo", mSetToNewerVersionAction );
    mSetToNewerVersionAction->setShortcuts( KStandardShortcut::redo() );

    connect( mSetToNewerVersionAction, SIGNAL(triggered( bool )),
             SLOT(onSetToNewerVersionTriggered()) );
    connect( mSetToNewerVersionAction->menu(), SIGNAL( aboutToShow() ),
             SLOT(onNewerVersionMenuAboutToShow()) );
    connect( mSetToNewerVersionAction->menu(), SIGNAL(triggered( QAction* )),
             SLOT(onNewerVersionMenuTriggered( QAction* )) );

    setView( 0 );
}

void VersionController::setView( KAbstractView *view )
{
    if( mDocument ) mDocument->disconnect( this );

    mDocument = view ? view->document() : 0;
    mVersionControl = mDocument ? qobject_cast<KDE::If::Versionable*>( mDocument ) : 0;

    // TODO: what about readonly? Rely on the model or adhere ourself?
    if( mVersionControl )
    {
        connect( mDocument, SIGNAL(revertedToVersionIndex( int )), SLOT(onVersionIndexChanged( int )) );
        connect( mDocument, SIGNAL(headVersionChanged( int )), SLOT(onVersionIndexChanged( int )) );
    }

    if( mVersionControl )
        onVersionIndexChanged( mVersionControl->versionIndex() );
    else
    {
        mSetToOlderVersionAction->setEnabled( false );
        mSetToNewerVersionAction->setEnabled( false );
    }
}

void VersionController::onVersionIndexChanged( int versionIndex )
{
    const bool hasOlderVersions = ( versionIndex > 0 );
    mSetToOlderVersionAction->setEnabled( hasOlderVersions );
    if( hasOlderVersions )
        mSetToOlderVersionAction->setData( versionIndex-1 );

    const bool hasNewerVersions = ( versionIndex < (mVersionControl->versionCount()-1) );
    mSetToNewerVersionAction->setEnabled( hasNewerVersions );
    if( hasNewerVersions )
        mSetToNewerVersionAction->setData( versionIndex+1 );
}

void VersionController::onSetToOlderVersionTriggered()
{
    const int versionIndex = mSetToOlderVersionAction->data().toInt();
    mVersionControl->revertToVersionByIndex( versionIndex );
}

void VersionController::onSetToNewerVersionTriggered()
{
    const int versionIndex = mSetToNewerVersionAction->data().toInt();
    mVersionControl->revertToVersionByIndex( versionIndex );
}

void VersionController::onOlderVersionMenuAboutToShow()
{
    QMenu *menu = mSetToOlderVersionAction->menu();
    menu->clear();

    int menuEntries = 0;
    for( int versionIndex = mVersionControl->versionIndex();
         versionIndex > 0 && menuEntries < MaxMenuEntries;
         --versionIndex, ++menuEntries )
    {
        KDocumentVersionData versionData = mVersionControl->versionData( versionIndex );

        const QString changeComment = versionData.changeComment();

        const QString actionText = i18nc( "@action Undo: [change]", "Undo: %1", changeComment );

        QAction *action = menu->addAction( actionText );
        action->setData( versionIndex-1 );
    }
}

void VersionController::onNewerVersionMenuAboutToShow()
{
    QMenu *menu = mSetToNewerVersionAction->menu();
    menu->clear();

    int menuEntries = 0;
    for( int versionIndex = mVersionControl->versionIndex()+1;
         versionIndex < mVersionControl->versionCount() && menuEntries < MaxMenuEntries;
         ++versionIndex, ++menuEntries )
    {
        KDocumentVersionData versionData = mVersionControl->versionData( versionIndex );

        const QString changeComment = versionData.changeComment();

        const QString actionText = i18nc( "@action Redo: [change]", "Redo: %1", changeComment );

        QAction *action = menu->addAction( actionText );
        action->setData( versionIndex );
    }
}

void VersionController::onOlderVersionMenuTriggered( QAction *action )
{
    const int versionIndex = action->data().toInt();
    mVersionControl->revertToVersionByIndex( versionIndex );
}

void VersionController::onNewerVersionMenuTriggered( QAction *action )
{
    const int versionIndex = action->data().toInt();
    mVersionControl->revertToVersionByIndex( versionIndex );
}
