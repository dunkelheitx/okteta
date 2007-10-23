/***************************************************************************
                          kdocumentabstractfactory.h  -  description
                             -------------------
    begin                : Thu Nov 2 2006
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


#ifndef KDOCUMENTABSTRACTFACTORY_H
#define KDOCUMENTABSTRACTFACTORY_H

// Qt
#include <QtCore/QObject>

class KAbstractDocument;


class KDocumentAbstractFactory : public QObject
{
  Q_OBJECT

  public:
    virtual ~KDocumentAbstractFactory();

  public: // API to be implemented
    virtual KAbstractDocument *create() = 0;
};

inline KDocumentAbstractFactory::~KDocumentAbstractFactory() {}

#endif
