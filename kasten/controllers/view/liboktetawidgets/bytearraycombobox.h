/*
    This file is part of the Okteta Kasten module, part of the KDE project.

    Copyright 2006,2009 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef BYTEARRAYCOMBOBOX_H
#define BYTEARRAYCOMBOBOX_H

// Qt
#include <QtGui/QWidget>

class QString;
class QByteArray;


namespace Okteta
{
class ByteArrayComboBoxPrivate;


class ByteArrayComboBox : public QWidget
{
  Q_OBJECT

  public:
    // TODO: same as in kbytearrayvalidator.h, we need a global enum containing both value and char codings...
    // matching Okteta::ValueCoding
    enum Coding { InvalidCoding = -1, HexadecimalCoding=0, DecimalCoding=1, OctalCoding=2, BinaryCoding=3, CharCoding=4 };

  public:
    explicit ByteArrayComboBox( QWidget* parent = 0 );
    virtual ~ByteArrayComboBox();

  public: // set
    void setCharCodec( const QString& charCodecName );

  public: // get
    QByteArray data() const;
    int format() const;

  Q_SIGNALS:
    void dataChanged( const QByteArray& data );
    void formatChanged( int index );

  protected:
    ByteArrayComboBoxPrivate* const d_ptr;
  private:
    Q_DECLARE_PRIVATE( ByteArrayComboBox )
    Q_PRIVATE_SLOT( d_func(), void onFormatChanged( int index ) )
    Q_PRIVATE_SLOT( d_func(), void onDataChanged( const QString& data ) )
};

}

#endif