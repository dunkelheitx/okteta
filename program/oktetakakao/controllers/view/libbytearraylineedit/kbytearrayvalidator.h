/*
    This file is part of the Okteta Kakao module, part of the KDE project.

    Copyright 2006 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KBYTEARRAYVALIDATOR_H
#define KBYTEARRAYVALIDATOR_H


// Okteta core
#include <khe.h>
// Qt
#include <QtGui/QValidator>

namespace KHECore {
class ValueCodec;
class KCharCodec;
}


class KByteArrayValidator : public QValidator
{
  Q_OBJECT

  public:
    static const QStringList &codingNames();

  public:
    KByteArrayValidator( QObject *parent, int Coding, int CharCoding = KHECore::LocalEncoding );
    virtual ~KByteArrayValidator();

  public: // QValidator API
    virtual QValidator::State validate( QString &input, int &pos ) const;

  public:
    void setCoding( int Coding );
    void setCharCode( const QString &CodeName );

  public:
    QByteArray toByteArray( const QString &src ) const;
    QString toString( const QByteArray &src ) const;

  private:
    /**
     * Returns a string that is at least as long as @p destLen number of characters,
     * by adding zeroes to the left as necessary.
     *
     * e.g. zeroExtend( "32", 3 ) => "032"
     */
//     QString zeroExtend( const QString &src, int destLen ) const;

    KHECore::KCoding Coding;
    KHECore::ValueCodec *ValueCodec;
    KHECore::KCharCodec *CharCodec;
};

#endif
