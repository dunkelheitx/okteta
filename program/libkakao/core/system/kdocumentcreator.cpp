/***************************************************************************
                          kdocumentmanager.cpp  -  description
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


#include "kdocumentcreator.h"

// lib
#include "kabstractdocumentfactory.h"
#include "kdocumentmanager.h"


KDocumentCreator::KDocumentCreator( KDocumentManager *manager )
 : mManager( manager ) {}

void KDocumentCreator::setWidget( QWidget *widget )
{
    mWidget = widget;
}

void KDocumentCreator::setDocumentFactory( KAbstractDocumentFactory *factory )
{
    mFactory = factory;
}


void KDocumentCreator::createNew()
{
   KAbstractDocument *document = mFactory->create();
   mManager->addDocument( document );
}


KDocumentCreator::~KDocumentCreator()
{
//     delete mFactory;
}
