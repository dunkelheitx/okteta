/*
    This file is part of the Kakao Framework, part of the KDE project.

    Copyright 2006-2007 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "zoomcontroller.h"

// Kakao gui
#include <kizoomable.h>
#include <kabstractview.h>
// KDE
#include <KXmlGuiWindow>
#include <KLocale>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>


ZoomController::ZoomController( KXmlGuiWindow *MW )
 : MainWindow( MW ), ZoomObject( 0 ), ZoomControl( 0 )
{
    KActionCollection* ActionCollection = MainWindow->actionCollection();

    ZoomInAction = KStandardAction::zoomIn(   this, SLOT(zoomIn()),  ActionCollection );
    ZoomOutAction = KStandardAction::zoomOut( this, SLOT(zoomOut()), ActionCollection );

#if 0
    ZoomToAction = new KSelectAction( i18n("Zoom"), "viewmag", 0, ActionCollection, "zoomTo" );
    ZoomToAction->setEditable( true );
    QList<double> mags = DisplayOptions::normalMagnificationValues();
    QStringList translated;
    int idx = 0;
    int cur = 0;
    for ( QList<double>::iterator first = mags.begin(), last = mags.end();
	    first != last;
	    ++first ) {
	translated << i18nc( "zoom-factor (percentage)", "%1%", *first * 100.0 );
	if ( *first == 1.0 ) idx = cur;
	++cur;
    }
    ZoomToAction->setItems( translated );
    ZoomToAction->setCurrentItem( idx );
    connect( ZoomToAction, SIGNAL(triggered(  const QString & ) ), SLOT(zoomTo( const QString& ) ) );

    // TODO: name size relative to object or view? name object(variable) or view?
    // TODO: is this a sticking parameter?
    FitToWidthAction = new KAction( i18n( "&Fit to Width" ), ActionCollection, "fit_to_width" );
    connect( FitWidthAction, SIGNAL(triggered(bool) ), SLOT( fitToWidth() ));
    FitToHeightAction = new KAction( i18n( "&Fit to Height" ), ActionCollection, "fit_to_height" );
    connect( FitWidthAction, SIGNAL(triggered(bool) ), SLOT( fitToHeight() ));
    FitToSizeAction = new KAction( i18n( "&Fit to Size" ), ActionCollection, "fit_to_size" );
    connect( FitToSizeAction, SIGNAL(triggered(bool) ), SLOT( fitToSize() ));
#endif
    setView( 0 );
}

void ZoomController::setView( KAbstractView *View )
{
    if( ZoomObject ) ZoomObject->disconnect( this );

    ZoomControl = View ? qobject_cast<KDE::If::Zoomable *>( View ) : 0;
    ZoomObject = ZoomControl ? View : 0;

    if( ZoomControl )
    {
        ZoomLevel = ZoomControl->zoomLevel();
        connect( ZoomObject, SIGNAL(zoomLevelChanged( double )), SLOT(onZoomLevelChange( double )) );
    }

    const bool HasView = ( ZoomControl != 0 );
    ZoomInAction->setEnabled( HasView );
    ZoomOutAction->setEnabled( HasView );
}


void ZoomController::zoomIn()
{
    ZoomControl->setZoomLevel( ZoomLevel * 1.10 );
}

void ZoomController::zoomOut()
{
    ZoomControl->setZoomLevel( ZoomLevel / 1.10 );
}
#if 0
void ZoomController::zoomTo( const QString& nz )
{
    QString z = nz;
    double zoom;
    z.remove(  z.indexOf(  '%' ), 1 );
    zoom = KGlobal::locale()->readNumber(  z ) / 100;
    kDebug( 4500 ) << "ZOOM = "  << nz << ", setting zoom = " << zoom << endl;

    DisplayOptions options = miniWidget()->displayOptions();
    options.setMagnification( zoom );
    miniWidget()->setDisplayOptions( options );
    miniWidget()->redisplay();
    _mainWidget->setFocus();
    updateZoomActions();
}

void ZoomController::fitToWidth()
{
    if( pageView()->page() )
	miniWidget()->fitWidth( pageView()->viewport()->width() - 16 );
    // We subtract 16 pixels because of the page decoration.
    updateZoomActions();
}

void ZoomController::fitToSize()
{
    if( pageView()->page() )
	miniWidget()->fitWidthHeight( pageView()->viewport()->width() - 16,
					pageView()->viewport()->height() - 16 );
    updateZoomActions();
}
#endif
void ZoomController::onZoomLevelChange( double Level )
{
    ZoomLevel = Level;
}
