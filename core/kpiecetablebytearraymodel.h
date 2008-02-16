/***************************************************************************
                          kpiecetablebytearraymodel.h  -  description
                             -------------------
    begin                : Sat Feb 02 2008
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


#ifndef KHE_CORE_KPIECETABLEBYTEARRAYMODEL_H
#define KHE_CORE_KPIECETABLEBYTEARRAYMODEL_H


// lib
#include "kabstractbytearraymodel.h"
#include "kversionable.h"


namespace KHECore
{

/** 
  *@author Friedrich W. H. Kossebau
  */

class KHECORE_EXPORT KPieceTableByteArrayModel : public KAbstractByteArrayModel, public Versionable
{
    Q_OBJECT
    Q_INTERFACES( KHECore::Versionable )

    class Private;
    friend class Private;

  public:
//     KPieceTableByteArrayModel( const char *data, unsigned int size );
//     KPieceTableByteArrayModel( char *D, unsigned int size );
    /**
     * makes a deep copy if careForMemory is false
     * @param data
     * @param careForMemory 
     */
    KPieceTableByteArrayModel( const char *data, unsigned int size, bool careForMemory = true );
//     explicit KPieceTableByteArrayModel( const QByteArray &data );
    explicit KPieceTableByteArrayModel( unsigned int size = 0, char fillByte = '\0' );
//     explicit KPieceTableByteArrayModel( int size = 0careForMemory, int maxSize = -1 );
    virtual ~KPieceTableByteArrayModel();

  public: // KAbstractByteArrayModel API
    virtual char datum( unsigned int offset ) const;
    virtual int size() const;
    virtual bool isReadOnly() const;
    virtual bool isModified() const;

    virtual int insert( int at, const char *data, int length );
    virtual int remove( const KSection &section );
    virtual unsigned int replace( const KSection &before, const char *after, unsigned int afterLength );
    virtual bool swap( int firstStart, const KSection &secondSection );
    virtual int fill( const char fillChar, unsigned int from = 0, int length = -1 );
    virtual void setDatum( unsigned int offset, const char datum );

    virtual void setModified( bool modified = true );
    virtual void setReadOnly( bool isReadOnly = true );

//     virtual int indexOf( const char *searchString, int length, int from  = 0 ) const;
//     virtual int lastIndexOf( const char *searchString, int length, int from = -1 ) const;

  public: // Versionable API
    virtual int versionIndex() const;
    virtual int versionCount() const;
    virtual QString versionDescription( int versionIndex ) const;

  public: // set/action
    virtual void revertToVersionByIndex( int versionIndex );

  public:
//     void setMaxSize( int MS );
    /** sets whether the memory given by setData or in the constructor should be kept on resize
      */
//     void setKeepsMemory( bool keepMemory = true );
//     void setAutoDelete( bool autoDelete = true );
    void setData( const char *data, unsigned int size, bool careForMemory = true );
//     void signalContentsChanged( int i1, int i2 );

  public:
//     char *data() const;
//     int maxSize() const;
    /** returns whether the memory of the byte array is kept on resize */
//     bool keepsMemory() const;
//     bool autoDelete() const;

  Q_SIGNALS: // Versionable signals
    virtual void revertedToVersionIndex( int versionIndex );
    virtual void headVersionDescriptionChanged( const QString &versionDescription );
    virtual void headVersionChanged( int newHeadVersionIndex );

  protected:
    class Private * const d;
};

}

#endif
