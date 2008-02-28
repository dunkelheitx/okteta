/*
    This file is part of the Okteta Kakao module, part of the KDE project.

    Copyright 2007-2008 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef STRINGSEXTRACTTOOL_H
#define STRINGSEXTRACTTOOL_H


// Qt
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

class KByteArrayDocument;
namespace KHECore {
class KCharCodec;
}

class ContainedString
{
  public:
    ContainedString( const QString &string, int offset );

  public:
    void move( int offset );

  public:
    QString string() const;
    int offset() const;

  protected:
    QString mString;
    int mOffset;
};
inline QString ContainedString::string() const { return mString; }
inline int ContainedString::offset() const { return mOffset; }


/**
*/
class StringsExtractTool : public QObject
{
  Q_OBJECT

  public:
    StringsExtractTool();
    ~StringsExtractTool();

  public:
    void setDocument( KByteArrayDocument *document );
    void setCharCodec( const QString &codecName );

    void extract();

  public:
    QList<ContainedString> containedStringList() const;

  Q_SIGNALS:
    void stringsChanged();

  protected:
    KByteArrayDocument *mDocument;
    KHECore::KCharCodec *mCharCodec;

    QList<ContainedString> mContainedStringList;
    int mMinLength;
};
#endif
