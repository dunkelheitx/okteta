/*
    This file is part of the Okteta Core library, part of the KDE project.

    Copyright 2006,2008 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "ksectiontest.h"

// test object
#include <ksection.h>
// Qt
#include <QtTest/QtTest>

using namespace KHE;

// local variables
static const int Start = 15;
static const int End = 27;

static const int Width = End - Start + 1;


void KSectionTest::testConstructorByEnd()
{
  KSection Section( Start, End );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   End );
  QCOMPARE( Section.width(), Width );
}

void KSectionTest::testConstructorByWidth()
{
  KSection Section = KSection::fromWidth( Start, Width );

  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), End );
  QCOMPARE( Section.width(), Width );

  Section = KSection::fromWidth( Width );

  QCOMPARE( Section.start(), 0 );
  QCOMPARE( Section.end(), Width-1 );
  QCOMPARE( Section.width(), Width );
}

void KSectionTest::testSimpleConstructor()
{
  KSection Section;
  QVERIFY( Section.isEmpty() );
}

void KSectionTest::testCopyConstructor()
{
  const KSection OtherSection( Start, End );

  KSection Section( OtherSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   End );
  QCOMPARE( Section.width(), Width );
  QVERIFY( !Section.isEmpty() );
}


void KSectionTest::testSetGetStart()
{
  KSection Section;
  Section.setStart( Start );
  QCOMPARE( Section.start(), Start );
}

void KSectionTest::testSetGetEnd()
{
  KSection Section;
  Section.setEnd( End );
  QCOMPARE( Section.end(), End );
}

void KSectionTest::testSetGetWidth()
{
  KSection Section;
  // end, width by width
  Section.setStart( Start );
  Section.setEndByWidth( Width );
  QCOMPARE( Section.end(), End );
  QCOMPARE( Section.width(), Width );
  // start, width by width
  Section.setEnd( End );
  Section.setStartByWidth( Width );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.width(), Width );
}

void KSectionTest::testIsEmpty()
{
  KSection Section( Start, End );
  QVERIFY( !Section.isEmpty() );
  Section.unset();
  QVERIFY( Section.isEmpty() );
}

void KSectionTest::testCompare()
{
  KSection Section( Start, End );
  // same
  KSection OtherSection( Start, End );
  QVERIFY( Section == OtherSection );
  // different start
  OtherSection.set(Start+1, End);
  QVERIFY( !(Section == OtherSection) );
  // different end
  OtherSection.set(Start, End+1);
  QVERIFY( !(Section == OtherSection) );
}

void KSectionTest::testIncludes()
{
  KSection Section( Start, End );
  // at start
  int i = Section.start();
  QVERIFY( Section.includes(i) );
  // before start
  --i;
  QVERIFY( !Section.includes(i) );
  // at end
  i = Section.end();
  QVERIFY( Section.includes(i) );
  // behind end
  ++i;
  QVERIFY( !Section.includes(i) );
}


void KSectionTest::testLocalIndex()
{
    KSection section( Start, End );

    int localIndex = section.localIndex( Start );
    QCOMPARE( localIndex, 0 );

    localIndex = section.localIndex( End );
    QCOMPARE( localIndex, Width-1 );
}


void KSectionTest::testLocalSection()
{
    KSection section( Start, End );

    KSection localSection = section.localSection( KSection(Start,End) );
    QCOMPARE( localSection.start(), 0 );
    QCOMPARE( localSection.end(), Width-1 );

    localSection = section.localSection( KSection(Start+1,End) );
    QCOMPARE( localSection.start(), 1 );
    QCOMPARE( localSection.end(), Width-1 );

    localSection = section.localSection( KSection(Start,End-1) );
    QCOMPARE( localSection.start(), 0 );
    QCOMPARE( localSection.end(), Width-2 );

    localSection = section.localSection( KSection(Start+1,End-1) );
    QCOMPARE( localSection.start(), 1 );
    QCOMPARE( localSection.end(), Width-2 );
}


void KSectionTest::testSubSection()
{
    KSection section( Start, End );

    KSection subSection = section.subSection( KSection::fromWidth(Width) );
    QCOMPARE( subSection.start(), Start );
    QCOMPARE( subSection.end(), End );

    subSection = section.subSection( KSection(1,Width-1) );
    QCOMPARE( subSection.start(), Start+1 );
    QCOMPARE( subSection.end(), End );

    subSection = section.subSection( KSection(0,Width-2) );
    QCOMPARE( subSection.start(), Start );
    QCOMPARE( subSection.end(), End-1 );

    subSection = section.subSection( KSection(1,Width-2) );
    QCOMPARE( subSection.start(), Start+1 );
    QCOMPARE( subSection.end(), End-1 );
}


void KSectionTest::testAdaptToChange()
{
  // adaptToChange, same length, behind
  KSection Section( Start, End );
  KSection OtherSection( Section );
  Section.adaptToChange( End+1, Width, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, before
  Section.set( Start, End );
  OtherSection.set( Section );
  Section.adaptToChange( Start-2, 1, 1 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, -1,-1
  Section.set( Start-1, End-1 );
  OtherSection.set( Start-1, Start-1 );
  Section.adaptToChange( Start, Width, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, -1,0
  Section.set( Start-1, End );
  OtherSection.set( Section );
  Section.adaptToChange( Start, Width, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, -1,+1
  Section.set( Start-1, End+1 );
  OtherSection.set( Section );
  Section.adaptToChange( Start, Width, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, 0,-1
  Section.set( Start, End-1 );
  OtherSection.set( Section );
  Section.adaptToChange( Start, Width, Width );
  QVERIFY( !Section.isValid() );

  // adaptToChange, same length, 0,0
  Section.set( Start, End );
  OtherSection.set( Section );
  Section.adaptToChange( Start, Width, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, 0,+1
  Section.set( Start, End+1 );
  OtherSection.set( Section );
  Section.adaptToChange( Start, Width, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, same length, +1,-1
  Section.set( Start+1, End-1 );
  Section.adaptToChange( Start, Width, Width );
  QVERIFY( !Section.isValid() );

  // adaptToChange, same length, +1,0
  Section.set( Start+1, End );
  Section.adaptToChange( Start, Width, Width );
  QVERIFY( !Section.isValid() );

  // adaptToChange, same length, +1,+1
  Section.set( Start+1, End+1 );
  OtherSection.set( End+1, End+1 );
  Section.adaptToChange( Start, Width, Width );
  QCOMPARE( Section, OtherSection );


  // adaptToChange, less length, behind
  Section.set( Start, End );
  OtherSection.set( Section );
  Section.adaptToChange( End+1, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, before
  Section.set( Start, End );
  OtherSection.set( Start-2, End-2 );
  Section.adaptToChange( Start-Width, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, -1,-1
  Section.set( Start-1, End-1 );
  OtherSection.set( Start-1, Start-1 );
  Section.adaptToChange( Start, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, -1,0
  Section.set( Start-1, End );
  OtherSection.set( Start-1, End-2 );
  Section.adaptToChange( Start, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, -1,+1
  Section.set( Start-1, End+1 );
  OtherSection.set( Start-1, End-1 );
  Section.adaptToChange( Start, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, 0,-1
  Section.set( Start, End-1 );
  Section.adaptToChange( Start, Width, Width-2 );
  QVERIFY( !Section.isValid() );

  // adaptToChange, less length, 0,0
  Section.set( Start, End );
  OtherSection.set( Start, End-2 );
  Section.adaptToChange( Start, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, 0,+1
  Section.set( Start, End+1 );
  OtherSection.set( Start, End-1 );
  Section.adaptToChange( Start, Width, Width-2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, less length, +1,-1
  Section.set( Start+1, End-1 );
  Section.adaptToChange( Start, Width, Width-2 );
  QVERIFY( !Section.isValid() );

  // adaptToChange, less length, +1,0
  Section.set( Start+1, End );
  Section.adaptToChange( Start, Width, Width-2 );
  QVERIFY( !Section.isValid() );

  // adaptToChange, less length, +1,+1
  Section.set( Start+1, End+1 );
  OtherSection.set( End-1, End-1 );
  Section.adaptToChange( Start, Width, Width-2 );
  QCOMPARE( Section, OtherSection );


  // adaptToChange, greater length, behind
  Section.set( Start, End );
  OtherSection.set( Section );
  Section.adaptToChange( End+1, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, before
  Section.set( Start, End );
  OtherSection.set( Start+2, End+2 );
  Section.adaptToChange( Start-Width, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, -1,-1
  Section.set( Start-1, End-1 );
  OtherSection.set( Start-1, Start-1 );
  Section.adaptToChange( Start, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, -1,0
  Section.set( Start-1, End );
  OtherSection.set( Start-1, End+2 );
  Section.adaptToChange( Start, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, -1,+1
  Section.set( Start-1, End+1 );
  OtherSection.set( Start-1, End+3 );
  Section.adaptToChange( Start, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, 0,-1
  Section.set( Start, End-1 );
  Section.adaptToChange( Start, Width, Width+2 );
  QVERIFY( !Section.isValid() );

  // adaptToChange, greater length, 0,0
  Section.set( Start, End );
  OtherSection.set( Start, End+2 );
  Section.adaptToChange( Start, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, 0,+1
  Section.set( Start, End+1 );
  OtherSection.set( Start, End+3 );
  Section.adaptToChange( Start, Width, Width+2 );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, greater length, +1,-1
  Section.set( Start+1, End-1 );
  Section.adaptToChange( Start, Width, Width+2 );
  QVERIFY( !Section.isValid() );

  // adaptToChange, greater length, +1,0
  Section.set( Start+1, End );
  Section.adaptToChange( Start, Width, Width+2 );
  QVERIFY( !Section.isValid() );

  // adaptToChange, greater length, +1,+1
  Section.set( Start+1, End+1 );
  OtherSection.set( End+3, End+3 );
  Section.adaptToChange( Start, Width, Width+2 );
  QCOMPARE( Section, OtherSection );


  // adaptToChange, insert, behind
  Section.set( Start-1, Start-1 );
  OtherSection.set( Start-1, End );
  Section.adaptToChange( Start, 0, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, insert, middle
  Section.set( Start-1, Start );
  OtherSection.set( Start-1, End+1 );
  Section.adaptToChange( Start, 0, Width );
  QCOMPARE( Section, OtherSection );

  // adaptToChange, insert, before
  Section.set( Start, Start );
  OtherSection.set( End+1, End+1 );
  Section.adaptToChange( Start, 0, Width );
  QCOMPARE( Section, OtherSection );
}


void KSectionTest::testSplitAt()
{
  KSection Section( Start, End );

  // split at start
  KSection SplitSection = Section.splitAt( Start );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start );
  QCOMPARE( SplitSection.end(),   End );

  // split at one after start
  Section.set( Start, End );
  SplitSection = Section.splitAt( Start+1 );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), Start );
  QCOMPARE( SplitSection.start(), Start+1 );
  QCOMPARE( SplitSection.end(),   End );

  // split at mid
  const int Mid = (Start+End)/2;
  Section.set( Start, End );
  SplitSection = Section.splitAt( Mid );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), Mid-1 );
  QCOMPARE( SplitSection.start(), Mid );
  QCOMPARE( SplitSection.end(),   End );

  // split at one before end
  Section.set( Start, End );
  SplitSection = Section.splitAt( End );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), End-1 );
  QCOMPARE( SplitSection.start(), End );
  QCOMPARE( SplitSection.end(),   End );

  // split at start so the split is the full
  Section.set( Start, End );
  SplitSection = Section.splitAt( End+1 );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), End );
  QVERIFY( !SplitSection.isValid() );
}

void KSectionTest::testSplitAtLocal()
{
  KSection Section( Start, End );

  // split at start
  KSection SplitSection = Section.splitAtLocal( 0 );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start );
  QCOMPARE( SplitSection.end(),   End );

  // split at one after start
  Section.set( Start, End );
  SplitSection = Section.splitAtLocal( 1 );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), Start );
  QCOMPARE( SplitSection.start(), Start+1 );
  QCOMPARE( SplitSection.end(),   End );

  // split at mid
  const int Mid = Width/2;
  Section.set( Start, End );
  SplitSection = Section.splitAtLocal( Mid );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), Start+Mid-1 );
  QCOMPARE( SplitSection.start(), Start+Mid );
  QCOMPARE( SplitSection.end(),   End );

  // split at one before end
  Section.set( Start, End );
  SplitSection = Section.splitAtLocal( Width-1 );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), End-1 );
  QCOMPARE( SplitSection.start(), End );
  QCOMPARE( SplitSection.end(),   End );

  // split at start so the split is the full
  Section.set( Start, End );
  SplitSection = Section.splitAtLocal( Width );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), End );
  QVERIFY( !SplitSection.isValid() );
}

void KSectionTest::testRemove()
{
  const int Mid = (Start+End)/2;
  KSection Section( Start, End );

  // remove none at start
  KSection RemoveSection( Start, Start-1  );
  KSection SplitSection = Section.remove( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start );
  QCOMPARE( SplitSection.end(),   End );

  // remove one at start
  Section.set( Start, End );
  RemoveSection.set( Start, Start );
  SplitSection = Section.remove( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start+1 );
  QCOMPARE( SplitSection.end(),   End );

  // remove many at start
  Section.set( Start, End );
  RemoveSection.set( Start, Mid );
  SplitSection = Section.remove( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Mid+1 );
  QCOMPARE( SplitSection.end(),   End );

  // remove all except last
  Section.set( Start, End );
  RemoveSection.set( Start, End-1 );
  SplitSection = Section.remove( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), End );
  QCOMPARE( SplitSection.end(),   End );

  // remove at mid
  Section.set( Start, End );
  RemoveSection.set( Mid-1, Mid+1 );
  SplitSection = Section.remove( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), Mid-2 );
  QCOMPARE( SplitSection.start(), Mid+2 );
  QCOMPARE( SplitSection.end(),   End );

  // remove none at end
  Section.set( Start, End );
  RemoveSection.set( End+1, End );
  SplitSection = Section.remove( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   End );
  QVERIFY( !SplitSection.isValid() );

  // remove one at end
  Section.set( Start, End );
  RemoveSection.set( End, End );
  SplitSection = Section.remove( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   End-1 );
  QVERIFY( !SplitSection.isValid() );

  // remove many at end
  Section.set( Start, End );
  RemoveSection.set( Mid, End );
  SplitSection = Section.remove( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   Mid-1 );
  QVERIFY( !SplitSection.isValid() );

  // remove all except first
  Section.set( Start, End );
  RemoveSection.set( Start+1, End );
  SplitSection = Section.remove( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   Start );
  QVERIFY( !SplitSection.isValid() );

  // remove all
  Section.set( Start, End );
  RemoveSection.set( Start, End );
  SplitSection = Section.remove( RemoveSection );
  QVERIFY( !Section.isValid() );
  QVERIFY( !SplitSection.isValid() );
}

void KSectionTest::testRemoveLocal()
{
  const int Mid = Width/2;
  KSection Section( Start, End );

  // remove none at start
  KSection RemoveSection( 0, 0-1  );
  KSection SplitSection = Section.removeLocal( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start );
  QCOMPARE( SplitSection.end(),   End );

  // remove one at start
  Section.set( Start, End );
  RemoveSection.set( 0, 0 );
  SplitSection = Section.removeLocal( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start+1 );
  QCOMPARE( SplitSection.end(),   End );

  // remove many at start
  Section.set( Start, End );
  RemoveSection.set( 0, Mid );
  SplitSection = Section.removeLocal( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), Start+Mid+1 );
  QCOMPARE( SplitSection.end(),   End );

  // remove all except last
  Section.set( Start, End );
  RemoveSection.set( 0, Width-2 );
  SplitSection = Section.removeLocal( RemoveSection );
  QVERIFY( !Section.isValid() );
  QCOMPARE( SplitSection.start(), End );
  QCOMPARE( SplitSection.end(),   End );

  // remove at mid
  Section.set( Start, End );
  RemoveSection.set( Mid-1, Mid+1 );
  SplitSection = Section.removeLocal( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(), Start+Mid-2 );
  QCOMPARE( SplitSection.start(), Start+Mid+2 );
  QCOMPARE( SplitSection.end(),   End );

  // remove none at end
  Section.set( Start, End );
  RemoveSection.set( Width, Width-1 );
  SplitSection = Section.removeLocal( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   End );
  QVERIFY( !SplitSection.isValid() );

  // remove one at end
  Section.set( Start, End );
  RemoveSection.set( Width-1, Width-1 );
  SplitSection = Section.removeLocal( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   End-1 );
  QVERIFY( !SplitSection.isValid() );

  // remove many at end
  Section.set( Start, End );
  RemoveSection.set( Mid, Width-1 );
  SplitSection = Section.removeLocal( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   Start+Mid-1 );
  QVERIFY( !SplitSection.isValid() );

  // remove all except first
  Section.set( Start, End );
  RemoveSection.set( 1, Width-1 );
  SplitSection = Section.removeLocal( RemoveSection );
  QCOMPARE( Section.start(), Start );
  QCOMPARE( Section.end(),   Start );
  QVERIFY( !SplitSection.isValid() );

  // remove all
  Section.set( Start, End );
  RemoveSection.set( 0, Width-1 );
  SplitSection = Section.removeLocal( RemoveSection );
  QVERIFY( !Section.isValid() );
  QVERIFY( !SplitSection.isValid() );
}

void KSectionTest::testStartForInclude()
{
  // testStartForInclude, same length, start at start
  const KSection Section( Start, End );
  KSection OtherSection( Section );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, same length, start before start
  OtherSection.moveBy( -1 );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, same length, end behind end
  OtherSection.moveBy( 2 );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, smaller length, start at start
  OtherSection.set( Start, End-1 );
  int Diff = Section.width() - OtherSection.width();
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, smaller length, start before start
  OtherSection.moveBy( -1 );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, smaller length, end behind end
  OtherSection.moveBy( 2 );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start()-Diff );

/* // undefined in startForInclude
  // testStartForInclude, greater length, start at start
  OtherSection.set( Start, End+1 );
  Diff = Section.width() - OtherSection.width();
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, smaller length, start before start
  OtherSection.moveBy( -1 );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start() );

  // testStartForInclude, smaller length, end behind end
  OtherSection.moveBy( 2 );
  QCOMPARE( Section.startForInclude(OtherSection), OtherSection.start()-Diff );
*/
}


void KSectionTest::testIsJoinable()
{
  // testStartForInclude, same length, start at start
  const KSection Section( Start, End );
  KSection OtherSection( Section );
  QVERIFY( Section.isJoinable(OtherSection) );

  // adaptToChange, insert, before
  OtherSection.set( End+1, End+Width );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Overlapping, right shifted
  OtherSection.set( Start+1, End+1 );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Overlapping, left shifted
  OtherSection.set( Start-1, End-1 );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Overlapping, 1 includes 2
  OtherSection.set( Start+1, End-1 );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Overlapping, 2 includes 1
  OtherSection.set( Start-1, End+1 );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Overlapping, identic
  OtherSection.set( Section );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Coupled, first, then second
  OtherSection.set( End+1, End+Width );
  QVERIFY( Section.isJoinable(OtherSection) );

  // Coupled, second, then first
  QVERIFY( OtherSection.isJoinable(Section) );

  // NonOverlapping, first, then second
  OtherSection.set( End+2, End+Width+1 );
  QVERIFY( !Section.isJoinable(OtherSection) );

  // NonOverlapping, second, then first
  QVERIFY( !OtherSection.isJoinable(Section) );
}


QTEST_MAIN( KSectionTest )
