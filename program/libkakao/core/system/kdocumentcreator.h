/***************************************************************************
                          kdocumentmanager.h  -  description
                             -------------------
    begin                : Fri Jun 2 2006
    copyright            : 2006 by Friedrich W. H. Kossebau
    email                : kossebau@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License           *
 *   version 2 as published by the Free Software Foundation.               *
 *                                                                         *
 ***************************************************************************/


#ifndef KDOCUMENTCREATOR_H
#define KDOCUMENTCREATOR_H

// lib
#include "kabstractdocument.h"
// Qt
#include <QtCore/QObject>

class QWidget;
class KUrl;
class KAbstractDocumentFactory;
class KDocumentManager;

class KDocumentCreator : public QObject
{
  Q_OBJECT

  public:
    explicit KDocumentCreator( KDocumentManager *manager );
    virtual ~KDocumentCreator();

  public:
    void createNew();

  public:
    void setDocumentFactory( KAbstractDocumentFactory *factory );
    void setWidget( QWidget *widget );

//  protected:
//    virtual KAbstractDocument* createDocument();

  protected:
    // unless there is a singleton
    KDocumentManager *mManager;
    // used for dialogs, TODO: create (or use?) global instance for this
    QWidget *mWidget;

    // temporary hack: hard coded factory for byte arrays
    KAbstractDocumentFactory *mFactory;
};

#endif
