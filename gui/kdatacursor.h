/*
    This file is part of the Okteta Gui library, part of the KDE project.

    Copyright 2003 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KHE_UI_KDATACURSOR_H
#define KHE_UI_KDATACURSOR_H

// lib
#include "kcoord.h"
// Qt
#include <QtCore/QList>

namespace KHE {
class ReplacementScope;
}

namespace KHEUI
{

class KDataLayout;


/**@short navigates through the buffer in an abstract way, based on the layout
  *
  * The cursor is allowed to access every coord that has content as
  * described in the layout. It holds the coord of the actual position
  * and the according index in the data array.
  *
  * To enable the cursor to be placed behind the last position in a line
  * (e.g, to mark all data in the line without placing the cursor to the
  * beginning of the next line) there is a flag Behind that should be read
  * as that the real index the cursor is at is the current one + 1
  * (as returned by realIndex())
  *
  * For appending new data to the buffer there is also the need to be able
  * to place the cursor at a position behind the last byte. This can be
  * enabled by calling setAppendPosEnabled(true). If the cursor is placed to
  * this position it gets the (real) index of the last byte + 1. As this
  * index does not point to an existing byte validIndex() returns -1.
  * Check for atAppendPos() to see whether cursor is at this position.
  *
  * If the buffer is empty there is no navigation possible, of course.
  * The cursor will be placed to coord 0/0 with index 1, Behind=false.
  *
  *@author Friedrich W. H. Kossebau
  */
class KDataCursor
{
  public:
    explicit KDataCursor( const KDataLayout *L );
    ~KDataCursor();


  public:
    bool operator==( const KDataCursor &c ) const;
    bool operator!=( const KDataCursor &c ) const { return !(*this == c); }

  public: // modificator
    void setAppendPosEnabled( bool APE=true );

  public: // state value access
    /** the index that is drawn at the actual coord */
    int index() const;
    /** the pos of the actual coord */
    int pos() const;
    /** the line of the actual coord */
    int line() const;
    /** the actual coord */
    KCoord coord() const;
    /** true if the cursor is located to the right of the actual coord but still shown at the coord */
    bool isBehind() const;
    /** returns the real index. That is if the cursor is tagged as "behind" the current index
      * it's real index is the next one.
      * Attention: this could be outside the data's range if the cursor is behind the last byte!
      */
    int realIndex() const;
    /** returns the true index if it is valid index that is it is inside the data's range.
      * Otherwise -1 is returned
      */
    int validIndex() const;

    //bool isValid() const;
    bool appendPosEnabled() const;

  public: // index calculation service
    /** returns the index at the start of the cursor's line */
    int indexAtLineStart() const;
    /** returns the index at the end of the cursor's line */
    int indexAtLineEnd() const;

  public: // navigation commands
    void gotoIndex( int I );
    void gotoCoord( const KCoord &C );
    void gotoCIndex( int I );
    void gotoCCoord( const KCoord &C );
    void gotoRealIndex();

    void gotoPreviousByte();
    void gotoNextByte();
    void gotoPreviousByte( int D );
    void gotoNextByte( int D );
    void gotoNextByteInLine();
    void gotoUp();
    void gotoDown();
    void gotoLineStart();
    void gotoLineEnd();
    void gotoStart();
    void gotoEnd();
    void gotoPageUp();
    void gotoPageDown();

    /** puts the cursor behind the actual position if it isn't already*/
    void stepBehind();
    void updateCoord();
    void adaptToChange( int Pos, int RemovedLength, int InsertedLength );
    void adaptToChange( const QList<KHE::ReplacementScope> &replacementList, int oldLength );


  public: // logical state access
    bool atStart() const;
    bool atEnd() const;
    /** could only be true in InsertMode: Cursor is behind the last byte */
    bool atAppendPos() const;
    bool atLineStart() const;
    bool atLineEnd() const;


  protected:
    /** if newpos allowed steps at a coord behind the last existing
      * or, if that is at a line end, behind the line
      * does not check for empty content!
      */
    void stepToEnd();

  private:
    /** layout, tells how the column is organized  */
    const KDataLayout *Layout;

    /** Position in buffer */
    int Index;
    /** Position and Line */
    KCoord Coord;

    /** tells whether the cursor is actually behind the actual position.
      * This is used for selection to the end of a line or of the whole buffer.
      */
    bool Behind : 1;

    /** tells whether there could be a position behind the end of the layout */
    bool AppendPosEnabled : 1;
};


inline int KDataCursor::index()          const { return Index; }
inline int KDataCursor::pos()            const { return Coord.pos(); }
inline int KDataCursor::line()           const { return Coord.line(); }
inline KCoord KDataCursor::coord() const { return Coord; }
inline bool KDataCursor::isBehind()      const { return Behind; }
inline int KDataCursor::realIndex()      const { return Behind ? Index + 1 : Index; }

inline void KDataCursor::stepBehind() { Behind = true; }

//inline bool KDataCursor::isValid()  const { return Index != -1; }

}

#endif
