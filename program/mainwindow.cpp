/*
    This file is part of the Okteta program, made within the KDE community.

    Copyright 2006-2011 Friedrich W. H. Kossebau <kossebau@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

// program
#include "program.h"
// tmp
#include <bytearrayviewprofilemanager.h>
// tools
#include <stringsextracttoolview.h>
#include <stringsextracttool.h>
#include <infotoolview.h>
#include <infotool.h>
#include <filtertoolview.h>
#include <filtertool.h>
#include <charsetconversiontoolview.h>
#include <charsetconversiontool.h>
#include <checksumtoolview.h>
#include <checksumtool.h>
#include <documentinfotoolview.h>
#include <documentinfotool.h>
#include <poddecodertoolview.h>
#include <poddecodertool.h>
#include <bytetabletoolview.h>
#include <bytetabletool.h>
#include <bookmarkstoolview.h>
#include <bookmarkstool.h>

#include <structtoolview.h>
// see comment in kasten/controllers/CMakeLists.txt
#include <view/structures/structtool.h>

// Kasten tools
#include <versionviewtoolview.h>
#include <versionviewtool.h>
#include <filesystembrowsertoolview.h>
#include <filesystembrowsertool.h>
#include <documentstoolview.h>
#include <documentstool.h>
#include <terminaltoolview.h>
#include <terminaltool.h>
// controllers
#include <overwriteonlycontroller.h>
#include <overwritemodecontroller.h>
#include <gotooffsetcontroller.h>
#include <selectrangecontroller.h>
#include <searchcontroller.h>
#include <replacecontroller.h>
#include <bookmarkscontroller.h>
#include <printcontroller.h>
#include <viewconfigcontroller.h>
#include <viewmodecontroller.h>
#include <viewstatuscontroller.h>
#include <viewprofilecontroller.h>
#include <viewprofilesmanagecontroller.h>
// Kasten controllers
#include <modifiedbarcontroller.h>
#include <readonlycontroller.h>
#include <readonlybarcontroller.h>
#include <creatorcontroller.h>
#include <loadercontroller.h>
#include <closecontroller.h>
#include <setremotecontroller.h>
#include <synchronizecontroller.h>
#include <clipboardcontroller.h>
#include <insertcontroller.h>
#include <copyascontroller.h>
#include <exportcontroller.h>
#include <versioncontroller.h>
#include <zoomcontroller.h>
#include <zoombarcontroller.h>
#include <selectcontroller.h>
#include <switchviewcontroller.h>
#include <viewlistmenucontroller.h>
#include <viewareasplitcontroller.h>
#include <toollistmenucontroller.h>
#include <fullscreencontroller.h>
/*#include <closecontroller.h>*/
#include <quitcontroller.h>
// Kasten gui
#include <multidocumentstrategy.h>
#include <modelcodecviewmanager.h>
#include <viewmanager.h>
#include <multiviewareas.h>
#include <statusbar.h>
// Kasten core
#include <modelcodecmanager.h>
#include <documentcreatemanager.h>
#include <documentsyncmanager.h>
#include <documentmanager.h>
// KF5
#include <KConfigGroup>
#include <KSharedConfig>
// Qt
#include <QUrl>
#include <QMimeData>


namespace Kasten
{

static const char LoadedUrlsKey[] = "LoadedUrls";

OktetaMainWindow::OktetaMainWindow( OktetaProgram* program )
  : ShellWindow( program->viewManager() ),
    mProgram( program )
{
    setObjectName( QStringLiteral("Shell") );

    // there is only one mainwindow, so have this show the document if requested
    connect( mProgram->documentManager(), SIGNAL(focusRequested(Kasten::AbstractDocument*)),
             SLOT(showDocument(Kasten::AbstractDocument*)) );
    connect( viewArea(), SIGNAL(dataOffered(const QMimeData*,bool&)),
             SLOT(onDataOffered(const QMimeData*,bool&)) );
    connect( viewArea(), SIGNAL(dataDropped(const QMimeData*)),
             SLOT(onDataDropped(const QMimeData*)) );
    connect( viewArea(), SIGNAL(closeRequest(QList<Kasten::AbstractView*>)),
             SLOT(onCloseRequest(QList<Kasten::AbstractView*>)) );

    // XXX: Workaround for Qt 4.4's lacking of proper handling of the initial layout of dock widgets
    //      This state is taken from an oktetarc where the docker constellation was configured by hand.
    //      Setting this state if none is present seems to work, but there's
    //      still the versioning problem to be accounted for.
    //      Hack borrowed from trunk/koffice/krita/ui/kis_view2.cpp:
    const QString mainWindowState = QStringLiteral(
"AAAA/wAAAAD9AAAAAwAAAAAAAADPAAACg/wCAAAAAvsAAAAiAEYAaQBsAGUAUwB5AHMAdABlAG0AQgByAG8AdwBzAGUAcgAAAABJAAACgwAAAB4BAAAF+wAAABIARABvAGMAdQBtAGUAbgB0AHMAAAAASQAAAmMAAABeAQAABQAAAAEAAAGcAAACXPwCAAAACPsAAAAUAFAATwBEAEQAZQBjAG8AZABlAHIAAAAAQgAAARMAAAB9AQAABfsAAAAUAFMAdAByAHUAYwB0AFQAbwBvAGwAAAAAQgAAAlwAAAB9AQAABfsAAAAQAFYAZQByAHMAaQBvAG4AcwAAAABNAAAAVgAAAF4BAAAF+wAAABgAQgBpAG4AYQByAHkARgBpAGwAdABlAHIAAAABegAAAM0AAAC8AQAABfsAAAAQAEMAaABlAGMAawBzAHUAbQAAAAF8AAAAywAAAL0BAAAF/AAAAREAAADlAAAAAAD////6AAAAAAEAAAAE+wAAABAAQwBoAGUAYwBrAFMAdQBtAQAAAAD/////AAAAAAAAAAD7AAAAEgBCAG8AbwBrAG0AYQByAGsAcwIAAALBAAAAPQAAAT8AAAFk+wAAAA4AUwB0AHIAaQBuAGcAcwAAAAAA/////wAAAQ8BAAAF+wAAAAgASQBuAGYAbwAAAAGRAAABTAAAAIUBAAAF+wAAABIAQgB5AHQAZQBUAGEAYgBsAGUAAAAAUwAAAjkAAAB9AQAABfsAAAAYAEQAbwBjAHUAbQBlAG4AdABJAG4AZgBvAAAAAEkAAAJjAAAA+wEAAAUAAAADAAAAAAAAAAD8AQAAAAH7AAAAEABUAGUAcgBtAGkAbgBhAGwAAAAAAP////8AAABPAQAABQAABBUAAAGLAAAABAAAAAQAAAAIAAAACPwAAAABAAAAAgAAAAEAAAAWAG0AYQBpAG4AVABvAG8AbABCAGEAcgEAAAAAAAAEBgAAAAAAAAAA");
    const char mainWindowStateKey[] = "State";
    KConfigGroup group( KSharedConfig::openConfig(), QStringLiteral("MainWindow") );
    if( !group.hasKey(mainWindowStateKey) )
        group.writeEntry( mainWindowStateKey, mainWindowState );

    setStatusBar( new Kasten::StatusBar(this) );

    setupControllers();
    setupGUI();

    // all controllers which use plugActionList have to do so after(!) setupGUI() or their entries will be removed
    // TODO: why is this so?
    // tmp
    addXmlGuiController( new ToolListMenuController(this,this) );
    addXmlGuiController( new ViewListMenuController(viewManager(),viewArea(),this) );
}

void OktetaMainWindow::setupControllers()
{
    MultiDocumentStrategy* const documentStrategy = mProgram->documentStrategy();
    ViewManager* const viewManager = this->viewManager();
    MultiViewAreas* const viewArea = this->viewArea();
    ModelCodecViewManager* const codecViewManager = viewManager->codecViewManager();
    DocumentManager* const documentManager = mProgram->documentManager();
    ModelCodecManager* const codecManager = documentManager->codecManager();
    DocumentSyncManager* const syncManager = documentManager->syncManager();
    // tmp
    ByteArrayViewProfileManager* const byteArrayViewProfileManager = mProgram->byteArrayViewProfileManager();

    // general, part of Kasten
    addXmlGuiController( new CreatorController(codecManager,
                                               documentStrategy,this) );
    addXmlGuiController( new LoaderController(documentStrategy,this) );
    addXmlGuiController( new SetRemoteController(syncManager,this) );
    addXmlGuiController( new SynchronizeController(syncManager,this) );
    addXmlGuiController( new ExportController(codecViewManager,
                                              codecManager,this) );
    addXmlGuiController( new CloseController(documentStrategy,this) );
    addXmlGuiController( new VersionController(this) );
    addXmlGuiController( new ReadOnlyController(this) );
    addXmlGuiController( new SwitchViewController(viewArea,this) );
    addXmlGuiController( new ViewAreaSplitController(viewManager,viewArea,this) );
    addXmlGuiController( new FullScreenController(this) );
    addXmlGuiController( new QuitController(0,this) );

    addXmlGuiController( new ZoomController(this) );
    addXmlGuiController( new SelectController(this) );
    addXmlGuiController( new ClipboardController(this) );
    addXmlGuiController( new InsertController(codecViewManager,
                                              codecManager,this) );
    addXmlGuiController( new CopyAsController(codecViewManager,
                                              codecManager,this) );

    addTool( new FileSystemBrowserToolView(new FileSystemBrowserTool( syncManager )) );
    addTool( new DocumentsToolView(new DocumentsTool( documentManager )) );
    addTool( new TerminalToolView(new TerminalTool( syncManager )) );
#ifndef NDEBUG
    addTool( new VersionViewToolView(new VersionViewTool()) );
#endif

    // Okteta specific
//     addXmlGuiController( new OverwriteOnlyController(this) );
    addXmlGuiController( new OverwriteModeController(this) );
    addXmlGuiController( new SearchController(this,this) );
    addXmlGuiController( new ReplaceController(this,this) );
    addXmlGuiController( new GotoOffsetController(viewArea,this) );
    addXmlGuiController( new SelectRangeController(viewArea,this) );
    addXmlGuiController( new BookmarksController(this) );
    addXmlGuiController( new PrintController(this) );
    addXmlGuiController( new ViewConfigController(this) );
    addXmlGuiController( new ViewModeController(this) );
    addXmlGuiController( new ViewProfileController(byteArrayViewProfileManager,this,this) );
    addXmlGuiController( new ViewProfilesManageController(this,byteArrayViewProfileManager,this) );

    Kasten::StatusBar* const bottomBar = static_cast<Kasten::StatusBar*>( statusBar() );
    addXmlGuiController( new ViewStatusController(bottomBar) );
    addXmlGuiController( new ModifiedBarController(bottomBar) );
    addXmlGuiController( new ReadOnlyBarController(bottomBar) );
    addXmlGuiController( new ZoomBarController(bottomBar) );

    addTool( new DocumentInfoToolView(new DocumentInfoTool(syncManager)) );
    addTool( new ChecksumToolView(new ChecksumTool()) );
    addTool( new FilterToolView(new FilterTool()) );
    addTool( new CharsetConversionToolView(new CharsetConversionTool()) );
    addTool( new StringsExtractToolView(new StringsExtractTool()) );
    addTool( new ByteTableToolView(new ByteTableTool()) );
    addTool( new InfoToolView(new InfoTool()) );
    addTool( new PODDecoderToolView(new PODDecoderTool()) );
    addTool( new StructToolView(new StructTool()) );
    addTool( new BookmarksToolView(new BookmarksTool()) );
}

bool OktetaMainWindow::queryClose()
{
    // TODO: query the document manager or query the view manager?
    return mProgram->documentManager()->canCloseAll();
}

void OktetaMainWindow::saveProperties( KConfigGroup& configGroup )
{
    DocumentManager* const documentManager = mProgram->documentManager();
    DocumentSyncManager* const syncManager = documentManager->syncManager();
    const QList<AbstractDocument*> documents = documentManager->documents();

    QStringList urls;
    foreach( AbstractDocument* document, documents )
        urls.append( syncManager->urlOf(document).url() );

    configGroup.writePathEntry( LoadedUrlsKey, urls );
}

void OktetaMainWindow::readProperties( const KConfigGroup& configGroup )
{
    const QStringList urls = configGroup.readPathEntry( LoadedUrlsKey, QStringList() );

    DocumentManager* const documentManager = mProgram->documentManager();
    DocumentSyncManager* const syncManager = documentManager->syncManager();
    DocumentCreateManager* const createManager = documentManager->createManager();
    foreach( const QUrl& url, urls )
    {
        if( url.isEmpty() )
            createManager->createNew();
        else
            syncManager->load( url );
        // TODO: set view to offset
        // if( offset != -1 )
    }
}

void OktetaMainWindow::onDataOffered( const QMimeData* mimeData, bool& accept )
{
    accept = mimeData->hasUrls()
             || mProgram->documentManager()->createManager()->canCreateNewFromData( mimeData );
}

void OktetaMainWindow::onDataDropped( const QMimeData* mimeData )
{
    const QList<QUrl> urls = mimeData->urls();

    DocumentManager* const documentManager = mProgram->documentManager();
    if( ! urls.isEmpty() )
    {
        DocumentSyncManager* const syncManager = documentManager->syncManager();

        foreach( const QUrl& url, urls )
            syncManager->load( url );
    }
    else
        documentManager->createManager()->createNewFromData( mimeData, true );
}

void OktetaMainWindow::onCloseRequest( const QList<Kasten::AbstractView*>& views )
{
    // group views per document
    QHash<AbstractDocument*,QList<AbstractView*> > viewsToClosePerDocument;
    foreach( AbstractView* view, views )
    {
        AbstractDocument* document = view->findBaseModel<AbstractDocument*>();
        viewsToClosePerDocument[document].append( view );
    }

    // find documents which lose all views
    const QList<AbstractView*> allViews = viewManager()->views();
    foreach( AbstractView* view, allViews )
    {
        AbstractDocument* document = view->findBaseModel<AbstractDocument*>();
        QHash<AbstractDocument*,QList<AbstractView*> >::Iterator it =
            viewsToClosePerDocument.find( document );

        if( it != viewsToClosePerDocument.end() )
        {
            const QList<AbstractView*>& viewsOfDocument = it.value();
            const bool isAnotherView = ! viewsOfDocument.contains( view );
            if( isAnotherView )
                viewsToClosePerDocument.erase( it );
        }
    }

    const QList<AbstractDocument*> documentsWithoutViews = viewsToClosePerDocument.keys();

    DocumentManager* const documentManager = mProgram->documentManager();
    if( documentManager->canClose(documentsWithoutViews) )
    {
        viewManager()->removeViews( views );
        documentManager->closeDocuments( documentsWithoutViews );
    }
}


OktetaMainWindow::~OktetaMainWindow() {}

}
