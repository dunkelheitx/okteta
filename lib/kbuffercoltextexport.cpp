/***************************************************************************
                          kbuffercoltextexport.cpp  -  description
                             -------------------
    begin                : Sam Aug 30 2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/

#include <iostream>
// c specific
#include <string.h>
// app specific
#include "kbuffercolumn.h"
#include "kbufferlayout.h"
#include "kbuffercoltextexport.h"

using namespace KHE;

static const int DefaultByteSpacingWidth = 1;
static const int GroupSpacingWidth = 3;


KBufferColTextExport::KBufferColTextExport( const KBufferColumn* BufferColumn, char *D, KCoordRange CR, int ByteWidth )
 : Data( D ),
   CoordRange( CR )
{
  NoOfBytesPerLine = BufferColumn->layout()->noOfBytesPerLine();
  Pos = new int[NoOfBytesPerLine];

  // TODO: remove this hack and make it more general
  int ByteSpacingWidth = BufferColumn->byteSpacingWidth();
  if( ByteSpacingWidth > 0 )
    ByteSpacingWidth = DefaultByteSpacingWidth;

  int SpacingTrigger = BufferColumn->noOfGroupedBytes()-1;
  if( SpacingTrigger < 0 )
    SpacingTrigger = NoOfBytesPerLine; // ensures to never trigger the group spacing

  int N = 0;
  int p = 0;
  int gs = 0;
  int *P = Pos;
  for( ; P<&Pos[NoOfBytesPerLine]; ++P, ++p, ++gs )
  {
    *P = N;
    N += ByteWidth;

    // is there a space behind the actual byte (if it is not the last)?
    if( gs == SpacingTrigger )
    {
      N += GroupSpacingWidth;
      gs = -1;
    }
    else
      N += ByteSpacingWidth;
  }
  N -= (gs==0)?GroupSpacingWidth:ByteSpacingWidth;

  NoOfCharsPerLine = N;
}




KBufferColTextExport::~KBufferColTextExport()
{
  delete [] Pos;
}


int KBufferColTextExport::charsPerLine() const
{
  return NoOfCharsPerLine;
}


void KBufferColTextExport::printFirstLine( char **T, int Line ) const
{
  PrintLine = Line;
  print( T );
}


void KBufferColTextExport::printNextLine( char **T ) const
{
  print( T );
}


void KBufferColTextExport::print( char **T ) const
{
  memset( *T, ' ', NoOfCharsPerLine );
  ++PrintLine;
}

