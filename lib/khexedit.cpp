/***************************************************************************
                          khexedit.cpp  -  description
                             -------------------
    begin                : Die Mai 13 2003
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

 
//#include <kdebug.h>

// c specific
#include <stdlib.h>
//#include <limits.h>
// c++ specific
//#include <limits>
// qt specific
#include <qstyle.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qapplication.h>
// kde specific
#ifndef QT_ONLY
#include <kglobalsettings.h>
#endif
// lib specific
#include "kdatabuffer.h"
#include "koffsetcolumn.h"
#include "kvaluecolumn.h"
#include "kcharcolumn.h"
#include "kbordercolumn.h"
#include "kbuffercursor.h"
#include "kbufferlayout.h"
#include "kbufferranges.h"
#include "kbufferdrag.h"
#include "kcursor.h"
#include "kbytecodec.h"
#include "kcharcodec.h"
#include "khexedit.h"

using namespace KHE;

// zooming is done in steps of font size points
static const int DefaultZoomStep = 1;
static const int DefaultStartOffset = 0;//5;
static const int DefaultFirstLineOffset = 0;
static const int DefaultNoOfBytesPerLine =  16;
static const KHexEdit::KResizeStyle DefaultResizeStyle = KHexEdit::FullSizeUsage;
static const KHexEdit::KEncoding DefaultEncoding = KHexEdit::LocalEncoding;
static const int DefaultScrollTimerPeriod = 100;
static const int InsertCursorWidth = 2;



KHexEdit::KHexEdit( KDataBuffer *Buffer, QWidget *Parent, const char *Name, WFlags Flags )
 : KColumnsView( Parent, Name, Flags ),
   DataBuffer( Buffer ),
   BufferLayout( new KBufferLayout(DefaultNoOfBytesPerLine,DefaultStartOffset,0) ),
   BufferCursor( new KBufferCursor(BufferLayout) ),
   BufferRanges( new KBufferRanges(BufferLayout) ),
   CursorBlinkTimer( new QTimer(this) ),
   ScrollTimer( new QTimer(this) ),
   DragStartTimer( new QTimer(this) ),
   TrippleClickTimer( new QTimer(this) ),
   CursorPixmaps( new KCursor() ),
   ByteBuffer( new char[KByteCodec::MaxCodingWidth+1] ),
   Codec( 0 ),
   ClipboardMode( QClipboard::Clipboard ),
   ResizeStyle( DefaultResizeStyle ),
   Encoding( MaxEncodingId ), // forces update
   TabChangesFocus( false ),
   ReadOnly( false ),
//    Modified( false ),
   OverWriteOnly( false ),
   OverWrite( true ),
   MousePressed( false ),
   InDoubleClick( false ),
   InDnD( false ),
   DragStartPossible( false ),
   CursorPaused( false ),
   BlinkCursorVisible( false ),
   InZooming( false ),
   InEditMode( false ),
   EditModeByInsert( false ),
   d( 0 )
{
  // initalize layout
  if( DataBuffer )
    BufferLayout->setLength( DataBuffer->size() );
  BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() );

  // creating the columns in the needed order
  OffsetColumn =       new KOffsetColumn( this, DefaultFirstLineOffset, DefaultNoOfBytesPerLine, KOffsetFormat::Hexadecimal );
  FirstBorderColumn =  new KBorderColumn( this );
  ValueColumn =        new KValueColumn( this, DataBuffer, BufferLayout, BufferRanges );
  SecondBorderColumn = new KBorderColumn( this );
  CharColumn =         new KCharColumn( this, DataBuffer, BufferLayout, BufferRanges );

  // select the active column
  ActiveColumn = &charColumn();
  InactiveColumn = &valueColumn();

  setEncoding( DefaultEncoding );

#ifdef QT_ONLY
  QFont FixedFont( "fixed", 10 );
  FixedFont.setFixedPitch( true );
  setFont( FixedFont );
#else
  setFont( KGlobalSettings::fixedFont() );
#endif

  // get the full control
  viewport()->setFocusProxy( this );
  viewport()->setFocusPolicy( WheelFocus );

  viewport()->installEventFilter( this );
  installEventFilter( this );

  connect( CursorBlinkTimer, SIGNAL(timeout()), this, SLOT(blinkCursor()) );
  connect( ScrollTimer,      SIGNAL(timeout()), this, SLOT(autoScrollTimerDone()) );
  connect( DragStartTimer,   SIGNAL(timeout()), this, SLOT(startDrag()) );

  viewport()->setAcceptDrops( true );
}


KHexEdit::~KHexEdit()
{
  delete [] ByteBuffer;
}


int KHexEdit::noOfBytesPerLine()               const { return BufferLayout->noOfBytesPerLine(); }
int KHexEdit::firstLineOffset()                const { return OffsetColumn->firstLineOffset(); }
int KHexEdit::startOffset()                    const { return BufferLayout->startOffset(); }
KHexEdit::KResizeStyle KHexEdit::resizeStyle() const { return ResizeStyle; }
KHexEdit::KCoding KHexEdit::coding()           const { return (KHexEdit::KCoding)valueColumn().coding(); }
KPixelX KHexEdit::byteSpacingWidth()           const { return valueColumn().byteSpacingWidth(); }
int KHexEdit::noOfGroupedBytes()               const { return valueColumn().noOfGroupedBytes(); }
KPixelX KHexEdit::groupSpacingWidth()          const { return valueColumn().groupSpacingWidth(); }
KPixelX KHexEdit::binaryGapWidth()             const { return valueColumn().binaryGapWidth(); }
bool KHexEdit::isOverwriteMode()               const { return OverWrite; }
bool KHexEdit::isOverwriteOnly()               const { return OverWriteOnly; }
bool KHexEdit::isReadOnly()                    const { return ReadOnly; }
bool KHexEdit::isModified()                    const { return DataBuffer->isModified(); }
bool KHexEdit::tabChangesFocus()               const { return TabChangesFocus; }
bool KHexEdit::showUnprintable()               const { return charColumn().showUnprintable(); }
QChar KHexEdit::substituteChar()               const { return charColumn().substituteChar(); }
QChar KHexEdit::undefinedChar()                const { return charColumn().undefinedChar(); }
KHexEdit::KEncoding KHexEdit::encoding()       const { return (KHexEdit::KEncoding)Encoding; }

int KHexEdit::cursorPosition() const { return BufferCursor->index(); }
bool KHexEdit::isCursorBehind() const { return BufferCursor->isBehind(); }
KHexEdit::KBufferColumnId KHexEdit::cursorColumn() const
{ return static_cast<KHE::KValueColumn *>( ActiveColumn ) == &valueColumn()? ValueColumnId : CharColumnId; }

void KHexEdit::setOverwriteOnly( bool OO )    { OverWriteOnly = OO; if( OverWriteOnly ) setOverwriteMode( true ); }
void KHexEdit::setModified( bool M )          { DataBuffer->setModified(M); }
void KHexEdit::setTabChangesFocus( bool TCF ) { TabChangesFocus = TCF; }
void KHexEdit::setFirstLineOffset( int FLO )  { OffsetColumn->setFirstLineOffset( FLO ); }

bool KHexEdit::offsetColumnVisible() const { return offsetColumn().isVisible(); }
int KHexEdit::visibleBufferColumns() const
{ return (valueColumn().isVisible() ? ValueColumnId : 0) | (charColumn().isVisible() ? CharColumnId : 0); }


void KHexEdit::setOverwriteMode( bool OM )
{
  if( (OverWriteOnly && !OM) || (OverWrite == OM) )
    return;

  OverWrite = OM;

  // affected:
  // cursor shape
  bool ChangeCursor = !(CursorPaused || InEditMode);
  if( ChangeCursor )
    pauseCursor();

  BufferCursor->setAppendPosEnabled( !OverWrite );

  if( ChangeCursor )
    unpauseCursor();

  emit cutAvailable( !OverWrite && BufferRanges->hasSelection() );
}


void KHexEdit::setDataBuffer( KDataBuffer *B )
{
  //pauseCursor();
  InEditMode = false;
  CursorPaused = true;

  DataBuffer = B;
  valueColumn().set( DataBuffer );
  charColumn().set( DataBuffer);

  // affected:
  // length -> no of lines -> width
  BufferLayout->setLength( DataBuffer->size() );
  adjustLayoutToSize();

  // ensure that the widget is readonly if the buffer is
  if( DataBuffer->isReadOnly() && !isReadOnly() )
    setReadOnly( true );

  updateView();
  BufferCursor->gotoStart();
  ensureCursorVisible();
  unpauseCursor();
}


void KHexEdit::setStartOffset( int SO )
{
  if( !BufferLayout->setStartOffset(SO) )
    return;

  pauseCursor();
  // affects:
  // the no of lines -> width
  adjustLayoutToSize();

  updateView();

  BufferCursor->updateCoord();
  ensureCursorVisible();
  unpauseCursor();
}


void KHexEdit::setReadOnly( bool RO )
{
  // don't set editor readwrite if databuffer is readonly
  ReadOnly = (DataBuffer && DataBuffer->isReadOnly()) ? true : RO;
}


void KHexEdit::setBufferSpacing( KPixelX ByteSpacing, int NoOfGroupedBytes, KPixelX GroupSpacing )
{
  if( !valueColumn().setSpacing(ByteSpacing,NoOfGroupedBytes,GroupSpacing) )
    return;

  updateViewByWidth();
}


void KHexEdit::setCoding( KCoding C )
{
  int OldCodingWidth = valueColumn().codingWidth();

  if( !valueColumn().setCoding((KHE::KCoding)C) )
    return;

  // no change in the width?
  if( valueColumn().codingWidth() == OldCodingWidth )
    updateColumn( valueColumn() );
  else
    updateViewByWidth();
}


void KHexEdit::setResizeStyle( KResizeStyle NewStyle )
{
  if( ResizeStyle == NewStyle )
    return;

  ResizeStyle = NewStyle;

  updateViewByWidth();
}


void KHexEdit::setNoOfBytesPerLine( int NoBpL )
{
  // if the number is explicitly set we expect a wish for no automatic resize
  ResizeStyle = NoResize;

  if( !BufferLayout->setNoOfBytesPerLine(NoBpL) )
    return;
  updateViewByWidth();
}


void KHexEdit::setByteSpacingWidth( int/*KPixelX*/ BSW )
{
  if( !valueColumn().setByteSpacingWidth(BSW) )
    return;
  updateViewByWidth();
}

void KHexEdit::setNoOfGroupedBytes( int NoGB )
{
  if( !valueColumn().setNoOfGroupedBytes(NoGB) )
    return;
  updateViewByWidth();
}


void KHexEdit::setGroupSpacingWidth( int/*KPixelX*/ GSW )
{
  if( !valueColumn().setGroupSpacingWidth(GSW) )
    return;
  updateViewByWidth();
}


void KHexEdit::setBinaryGapWidth( int/*KPixelX*/ BGW )
{
  if( !valueColumn().setBinaryGapWidth(BGW) )
    return;
  updateViewByWidth();
}


void KHexEdit::setSubstituteChar( QChar SC )
{
  if( !charColumn().setSubstituteChar(SC) )
    return;
  pauseCursor();
  updateColumn( charColumn() );
  unpauseCursor();
}

void KHexEdit::setUndefinedChar( QChar UC )
{
  if( !charColumn().setUndefinedChar(UC) )
    return;
  pauseCursor();
  updateColumn( charColumn() );
  unpauseCursor();
}

void KHexEdit::setShowUnprintable( bool SU )
{
  if( !charColumn().setShowUnprintable(SU) )
    return;
  pauseCursor();
  updateColumn( charColumn() );
  unpauseCursor();
}


void KHexEdit::setEncoding( KEncoding C )
{
  if( Encoding == C )
    return;

  KCharCodec *NC = KCharCodec::create( (KHE::KEncoding)C );
  if( NC == 0 )
    return;

  valueColumn().setCodec( NC );
  charColumn().setCodec( NC );

  delete Codec;
  Codec = NC;
  Encoding = C;

  updateColumn( valueColumn() );
  updateColumn( charColumn() );
}


void KHexEdit::fontChange( const QFont &OldFont )
{
  QScrollView::fontChange( OldFont );

  if( !InZooming )
    DefaultFontSize = font().pointSize();

  // get new values
  QFontMetrics FM( fontMetrics() );
  KPixelX DigitWidth = FM.maxWidth();
  KPixelY DigitBaseLine = FM.ascent();

  setLineHeight( FM.height() );

  // update all dependant structures
  BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() );

  offsetColumn().setMetrics( DigitWidth, DigitBaseLine );
  valueColumn().setMetrics( DigitWidth, DigitBaseLine );
  charColumn().setMetrics( DigitWidth, DigitBaseLine );

  updateViewByWidth();
}


void KHexEdit::updateViewByWidth()
{
  pauseCursor();

  adjustToLayoutNoOfBytesPerLine();
  adjustLayoutToSize();

  updateView();

  BufferCursor->updateCoord();
  ensureCursorVisible();

  unpauseCursor();
}


void KHexEdit::zoomIn()         { zoomIn( DefaultZoomStep ); }
void KHexEdit::zoomOut()        { zoomOut( DefaultZoomStep ); }

void KHexEdit::zoomIn( int PointInc )
{
  InZooming = true;
  QFont F( font() );
  F.setPointSize( QFontInfo(F).pointSize() + PointInc );
  setFont( F );
  InZooming = false;
}

void KHexEdit::zoomOut( int PointDec )
{
  InZooming = true;
  QFont F( font() );
  F.setPointSize( QMAX( 1, QFontInfo(F).pointSize() - PointDec ) );
  setFont( F );
  InZooming = false;
}


void KHexEdit::zoomTo( int PointSize )
{
  InZooming = true;
  QFont F( font() );
  F.setPointSize( PointSize );
  setFont( F );
  InZooming = false;
}


void KHexEdit::unZoom()
{
  zoomTo( DefaultFontSize );
}


void KHexEdit::adjustLayoutToSize()
{
  // check whether there is a change with the numbers of fitting bytes per line
  if( ResizeStyle != NoResize )
  {
    int FittingBytesPerLine = fittingBytesPerLine( size() );

//     std::cout<<"FitBpL"<<FittingBytesPerLine<<std::endl;

    // changes?
    if( BufferLayout->setNoOfBytesPerLine(FittingBytesPerLine) )
      adjustToLayoutNoOfBytesPerLine();
  }

  setNoOfLines( BufferLayout->noOfLines() );
}


void KHexEdit::adjustToLayoutNoOfBytesPerLine()
{
  offsetColumn().setDelta( BufferLayout->noOfBytesPerLine() );
  valueColumn().resetXBuffer();
  charColumn().resetXBuffer();

  updateWidths();
}


void KHexEdit::setNoOfLines( int NewNoOfLines )
{
  KColumnsView::setNoOfLines( NewNoOfLines>1?NewNoOfLines:1 );
}


void KHexEdit::toggleOffsetColumn( bool Visible )
{
  bool OCVisible = offsetColumn().isVisible();
  // no change?
  if( OCVisible == Visible )
    return;

  offsetColumn().setVisible( Visible );
  FirstBorderColumn[0].setVisible( Visible );

  updateViewByWidth();
}


QSize KHexEdit::sizeHint() const
{
  return QSize( totalWidth(), totalHeight() );
}


QSize KHexEdit::minimumSizeHint() const
{
  // TODO: better minimal width (visibility!)
  return QSize( offsetColumn().visibleWidth()+FirstBorderColumn->visibleWidth()+SecondBorderColumn->visibleWidth()+valueColumn().byteWidth()+charColumn().byteWidth(),
                lineHeight() + noOfLines()>1? style().pixelMetric(QStyle::PM_ScrollBarExtent):0 );
}


void KHexEdit::resizeEvent( QResizeEvent *ResizeEvent )
{
  if( ResizeStyle != NoResize )
  {
    int FittingBytesPerLine = fittingBytesPerLine( ResizeEvent->size() );

    // changes?
    if( BufferLayout->setNoOfBytesPerLine(FittingBytesPerLine) )
    {
      setNoOfLines( BufferLayout->noOfLines() );
      updateViewByWidth();
    }
  }

  QScrollView::resizeEvent( ResizeEvent );

  BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() ); // TODO: doesn't work with the new size!!!
}


int KHexEdit::fittingBytesPerLine( const QSize &NewSize ) const
{
  KPixelX ReservedWidth = OffsetColumn->visibleWidth() + FirstBorderColumn->visibleWidth() + SecondBorderColumn->visibleWidth();

  // abstract framewidth as well as offset and border columns width
  int UsedbyFrameWidth = 2 * frameWidth();
  KPixelX FullWidth = NewSize.width() - UsedbyFrameWidth - ReservedWidth;

//  // no width left for resizeable columns? TODO: put this in resizeEvent
//  if( FullWidth < 0 )
//    return;

  KPixelY FullHeight = NewSize.height() - UsedbyFrameWidth;

  // check influence of dis-/appearing of the vertical scrollbar
  bool VerticalScrollbarIsVisible = verticalScrollBar()->isVisible();
  KPixelX ScrollbarExtent = style().pixelMetric( QStyle::PM_ScrollBarExtent );//verticalScrollBar()->width();

  KPixelX AvailableWidth = FullWidth;
  if( VerticalScrollbarIsVisible )
    AvailableWidth -= ScrollbarExtent;

  enum KMatchTrial { FirstRun, RerunWithScrollbarOn, TestWithoutScrollbar };
  KMatchTrial MatchRun = FirstRun;

  // prepare needed values
  KPixelX DigitWidth = valueColumn().digitWidth();
  KPixelX TextByteWidth = charColumn().isVisible() ? DigitWidth : 0;
  KPixelX HexByteWidth = valueColumn().isVisible() ? valueColumn().byteWidth() : 0;
  KPixelX ByteSpacingWidth = valueColumn().isVisible() ? valueColumn().byteSpacingWidth() : 0;
  KPixelX GroupSpacingWidth;
  int NoOfGroupedBytes = valueColumn().noOfGroupedBytes();
  // no grouping?
  if( NoOfGroupedBytes == 0 )
  {
    // faking grouping by 1
    NoOfGroupedBytes = 1;
    GroupSpacingWidth = 0;
  }
  else
    GroupSpacingWidth = valueColumn().isVisible() ? valueColumn().groupSpacingWidth() : 0;

  KPixelX HexByteGroupWidth =  NoOfGroupedBytes * HexByteWidth + (NoOfGroupedBytes-1)*ByteSpacingWidth;
  KPixelX TextByteGroupWidth = NoOfGroupedBytes * TextByteWidth;
  KPixelX TotalGroupWidth = HexByteGroupWidth + GroupSpacingWidth + TextByteGroupWidth;

  int FittingBytesPerLine;
  int WithScrollbarFittingBytesPerLine = 0;
  for(;;)
  {
//    std::cout << "matchWidth: " << FullWidth
//              << " (v:" << visibleWidth()
//              << ", f:" << frameWidth()
//              << ", A:" << AvailableWidth
//              << ", S:" << ScrollbarExtent
//              << ", R:" << ReservedWidth << ")" << std::endl;

    // calculate fitting groups per line
    int FittingGroupsPerLine = (AvailableWidth+GroupSpacingWidth) // fake spacing after last group
                               / TotalGroupWidth;

    // calculate the fitting bytes per line by groups
    FittingBytesPerLine = NoOfGroupedBytes * FittingGroupsPerLine;

    // not only full groups?
    if( ResizeStyle == FullSizeUsage && NoOfGroupedBytes > 1 )
    {
      if( FittingGroupsPerLine > 0 )
        AvailableWidth -= FittingGroupsPerLine*TotalGroupWidth; // includes additional spacing after last group

//        std::cout << "Left: " << AvailableWidth << "("<<HexByteWidth<<", "<<TextByteWidth<<")" << std::endl;

      if( AvailableWidth > 0 )
        FittingBytesPerLine += (AvailableWidth+ByteSpacingWidth) / (HexByteWidth+ByteSpacingWidth+TextByteWidth);

      // is there not even the space for a single byte?
      if( FittingBytesPerLine == 0 )
      {
        // ensure at least one byte per line
        FittingBytesPerLine = 1;
        // and
        break;
      }
    }
    // is there not the space for a single group?
    else if( FittingBytesPerLine == 0 )
    {
      // ensures at least one group
      FittingBytesPerLine = NoOfGroupedBytes;
      break;
    }

//    std::cout << "meantime: " << FittingGroupsPerLine << " (T:" << TotalGroupWidth
//              << ", h:" << HexByteGroupWidth
//              << ", t:" << TextByteGroupWidth
//              << ", s:" << GroupSpacingWidth << ") " <<FittingBytesPerLine<< std::endl;

    int NewNoOfLines = (BufferLayout->length()+BufferLayout->startOffset()+FittingBytesPerLine-1)
                       / FittingBytesPerLine;
    KPixelY NewHeight =  NewNoOfLines * LineHeight;

    if( VerticalScrollbarIsVisible )
    {
      if( MatchRun == TestWithoutScrollbar )
      {
        // did the test without the scrollbar fail, don't the data fit into the view?
        if( NewHeight>FullHeight )
          // reset to old calculated value
          FittingBytesPerLine =  WithScrollbarFittingBytesPerLine;
        break;
      }

      // a chance for to perhaps fit in height?
      if( FittingBytesPerLine <= BufferLayout->noOfBytesPerLine() )
      {
        // remember this trial's result and calc number of bytes with vertical scrollbar on
        WithScrollbarFittingBytesPerLine = FittingBytesPerLine;
        AvailableWidth = FullWidth;
        MatchRun = TestWithoutScrollbar;
//          std::cout << "tested without scrollbar..." << std::endl;
        continue;
      }
    }
    else
    {
      // doesn't it fit into the height anymore?
      if( NewHeight>FullHeight && MatchRun==FirstRun )
      {
        // need for a scrollbar has risen... ->less width, new calculation
        AvailableWidth = FullWidth - ScrollbarExtent;
        MatchRun = RerunWithScrollbarOn;
//          std::cout << "rerun with scrollbar on..." << std::endl;
        continue;
      }
    }

    break;
  }

  return FittingBytesPerLine;
}


bool KHexEdit::selectWord( /*unsigned TODO:change all unneeded signed into unsigned!*/ int Index )
{
  if( Index >= 0 && Index < BufferLayout->length()  )
  {
      KSection WordSection = DataBuffer->wordSection( Index );
      if( WordSection.isValid() )
      {
        pauseCursor();

        BufferRanges->setFirstWordSelection( WordSection );
        BufferCursor->gotoIndex( WordSection.end()+1 );
        repaintChanged();

        unpauseCursor();
        return true;
      }
   }
   return false;
}


void KHexEdit::selectAll( bool Select )
{
  pauseCursor( true );

  if( !Select )
    BufferRanges->removeSelection();
  else
  {
    BufferRanges->setSelection( KSection(0,BufferLayout->length()-1) );
    BufferCursor->gotoEnd();
  }

  repaintChanged();

  unpauseCursor();

  if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
  emit copyAvailable( BufferRanges->hasSelection() );
  emit selectionChanged();
  viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
}


bool KHexEdit::hasSelectedData() const
{
  return BufferRanges->hasSelection();
}


QByteArray KHexEdit::selectedData() const
{
  if( !BufferRanges->hasSelection() )
    return QByteArray();

  KSection Selection = BufferRanges->selection();
  QByteArray SD( Selection.width() );
  DataBuffer->copyTo( SD.data(), Selection.start(), Selection.width() );
  return SD;
}


KBufferDrag *KHexEdit::dragObject( QWidget *Parent ) const
{
  if( !BufferRanges->hasSelection() )
    return 0;

  const KValueColumn *HC;
  const KCharColumn *TC;
  KCoordRange Range;

  if( static_cast<KHE::KCharColumn *>( ActiveColumn ) == &charColumn() )
  {
    HC = 0;
    TC = 0;
  }
  else
  {
    HC = &valueColumn();
    TC = charColumn().isVisible() ? &charColumn() : 0;
    KSection S = BufferRanges->selection();
    Range.set( BufferLayout->coordOfIndex(S.start()),BufferLayout->coordOfIndex(S.end()) );
  }

  return new KBufferDrag( selectedData(), Range, OffsetColumn,HC,TC, charColumn().substituteChar(), Parent );
}


void KHexEdit::cut()
{
  if( isReadOnly() || OverWrite )
    return;

  KBufferDrag *Drag = dragObject();
  if( !Drag )
    return;

  QApplication::clipboard()->setData( Drag, ClipboardMode );

  removeSelectedData();
}


void KHexEdit::copy()
{
  KBufferDrag *Drag = dragObject();
  if( !Drag )
    return;

  QApplication::clipboard()->setData( Drag, ClipboardMode );
}


void KHexEdit::paste()
{
  if( isReadOnly() )
    return;

  QMimeSource *Source = QApplication::clipboard()->data( ClipboardMode );
  pasteFromSource( Source );
}


void KHexEdit::pasteFromSource( QMimeSource *Source )
{
  if( !Source || !KBufferDrag::canDecode(Source) )
    return;

  QByteArray Data;
  if( !KBufferDrag::decode(Source,Data) )
    return;

  if( !Data.isEmpty() )
    insert( Data );
}


void KHexEdit::insert( const QByteArray &D )
{
  if( InEditMode )
  {
    emit inputFailed();
    return;
  }

  pauseCursor();

  if( OverWrite )
  {
    if( BufferRanges->hasSelection() )
    {
      // replacing the selection:
      // we restrict the replacement to the minimum length of selection and input
      KSection Selection = BufferRanges->selection();
      Selection.restrictEndTo( Selection.start()+D.size()-1 );
      int W = DataBuffer->replace( Selection, D.data(), Selection.width() );
      BufferCursor->gotoCIndex( Selection.start()+W );
      BufferRanges->addChangedRange( Selection );
      BufferRanges->removeSelection();
    }
    else
    {
      // replacing the normal data, at least until the end
      KSection Section( BufferCursor->realIndex(), D.size(), false );
      Section.restrictEndTo( BufferLayout->length()-1 );
      if( Section.isValid() && !BufferCursor->isBehind() )
      {
        int W = DataBuffer->replace( Section, D.data(), Section.width() );
        BufferCursor->gotoNextByte( W );
        BufferRanges->addChangedRange( Section );
      }
      else
      {
        unpauseCursor();

        emit inputFailed();
        return;
      }
    }
  }
  else
  {
    if( BufferRanges->hasSelection() )
    {
      // replacing the selection
      KSection Selection = BufferRanges->selection();
      int OldLastIndex = BufferLayout->length() - 1;
      int W = DataBuffer->replace( Selection, D.data(), D.size() );
      updateLength();
      BufferCursor->gotoIndex( Selection.start() + W );
      if( W > 0 )
      {
        if( Selection.width() == (int)D.size() )
          BufferRanges->addChangedRange( Selection );
        else
        {
          int NewLastIndex = DataBuffer->size() - 1;
          BufferRanges->addChangedRange( Selection.start(), NewLastIndex>OldLastIndex?NewLastIndex:OldLastIndex );
        }
      }
      BufferRanges->removeSelection();
    }
    else
    {
      bool Appending = BufferCursor->atAppendPos();
      int OldIndex = BufferCursor->realIndex();
      int W = DataBuffer->insert( OldIndex, D.data(), D.size() );
      updateLength();
      if( W > 0 )
      {
        if( Appending )
          BufferCursor->gotoEnd();
        else
          BufferCursor->gotoNextByte( W );
        BufferRanges->addChangedRange( KSection(OldIndex,DataBuffer->size()-1) );
      }
      else
      {
        unpauseCursor();

        emit inputFailed();
        return;
      }
    }
  }
  repaintChanged();

  unpauseCursor();

  emit bufferChanged();
  emit selectionChanged();
}


void KHexEdit::removeSelectedData()
{
  if( isReadOnly() || OverWrite || InEditMode )
    return;

  pauseCursor();

  KSection Selection = BufferRanges->selection();

  BufferRanges->removeFurtherSelections();

  removeData( Selection );
  BufferRanges->removeSelection();

  repaintChanged();

  BufferCursor->gotoCIndex( Selection.start() );

  ensureCursorVisible();
//     clearUndoRedo();
  viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );

  unpauseCursor();

  emit bufferChanged();
  emit selectionChanged();
}


void KHexEdit::removeData( KSection Indizes )
{
//   if( undoEnabled )
//   {
//     checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
//     if( !undoRedoInfo.valid() )
//     {
//       doc->selectionStart( selNum, undoRedoInfo.id, undoRedoInfo.index );
//       undoRedoInfo.d->text = QString::null;
//     }
//     readFormats( c1, c2, undoRedoInfo.d->text, TRUE );
//   }

  int OldLastIndex = BufferLayout->length()-1;
  // do it!
  DataBuffer->remove( Indizes );
  updateLength();
  BufferRanges->addChangedRange( Indizes.start(), OldLastIndex );
}


void KHexEdit::updateLength()
{
  BufferLayout->setLength( DataBuffer->size() );
  setNoOfLines( BufferLayout->noOfLines() );
}


void KHexEdit::goOutsideByte( bool MoveToNext )
{
  pauseCursor( true );

  if( MoveToNext )
    BufferCursor->gotoNextByte();

  unpauseCursor();
}


bool KHexEdit::goInsideByte()
{
  if( InEditMode )
   return true;

  int ValidIndex = BufferCursor->validIndex();
  if( ValidIndex == -1 || !OverWrite || isReadOnly() || BufferCursor->isBehind() )
    return false;

  bool ColumnSwitch = (ActiveColumn == &charColumn());

  //
  InEditMode = true;
  EditModeByInsert = false;

  if( ColumnSwitch )
  {
    ActiveColumn = &valueColumn();
    InactiveColumn = &charColumn();
  }

  OldValue = EditValue = (unsigned char)DataBuffer->datum( ValidIndex );
  syncEditedByte();

  paintActiveCursor( true );
  if( ColumnSwitch )
    paintInactiveCursor( true );

  // put Cursor at the end (implemented later)
  return true;
}


bool KHexEdit::incByte()
{
  if( !InEditMode && !goInsideByte() )
    return false;
  if( EditValue < 255 )
  {
    ++EditValue;
    syncEditedByte();

    updateCursor();
    return true;
  }
  else
    return false;
}

bool KHexEdit::decByte()
{
  if( !InEditMode && !goInsideByte() )
    return false;
  if( EditValue > 0 )
  {
    --EditValue;
    syncEditedByte();

    updateCursor();// paintEditedByte( true );
    return true;
  }
  else
    return false;
}


void KHexEdit::syncEditedByte()
{
  valueColumn().codingFunction()( ByteBuffer, EditValue );
  DataBuffer->replace( BufferCursor->index(), 1, (char*)&EditValue, 1 );
}



void KHexEdit::clipboardChanged()
{
  // don't listen to selection changes
  disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0 );
  selectAll( false );
}


void KHexEdit::setCursorPosition( int Index, bool Behind )
{
  pauseCursor( true );

  BufferCursor->gotoCIndex( Index );
  if( Behind )
    BufferCursor->stepBehind();

  BufferRanges->removeSelection();
  if( BufferRanges->isModified() )
  {
    repaintChanged();

    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );

    if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
    emit copyAvailable( BufferRanges->hasSelection() );
    emit selectionChanged();
  }
  ensureCursorVisible();

  unpauseCursor();
}


void KHexEdit::showBufferColumns( int CCs )
{
  int Columns = visibleBufferColumns();

  // no changes or no column selected?
  if( CCs == Columns || !(CCs&( ValueColumnId | CharColumnId )) )
    return;

  valueColumn().setVisible( ValueColumnId & CCs );
  charColumn().setVisible( CharColumnId & CCs );
  SecondBorderColumn->setVisible( CCs == (ValueColumnId|CharColumnId) );

  // active column not visible anymore?
  if( !activeColumn().isVisible() )
  {
    KBufferColumn *H = ActiveColumn;
    ActiveColumn = InactiveColumn;
    InactiveColumn = H;
  }

  updateViewByWidth();
}


void KHexEdit::setCursorColumn( KBufferColumnId CC )
{
  // no changes or not visible?
  if( CC == cursorColumn()
      || (CC == ValueColumnId && !valueColumn().isVisible())
      || (CC == CharColumnId && !charColumn().isVisible()) )
    return;

  pauseCursor( true );

  if( CC == ValueColumnId )
  {
    ActiveColumn = &valueColumn();
    InactiveColumn = &charColumn();
  }
  else
  {
    ActiveColumn = &charColumn();
    InactiveColumn = &valueColumn();
  }
  ensureCursorVisible();
  unpauseCursor();
}

void KHexEdit::placeCursor( const QPoint &Point )
{
  resetInputContext();

  // switch active column if needed
  if( charColumn().isVisible() && Point.x() >= charColumn().x() )
  {
    ActiveColumn = &charColumn();
    InactiveColumn = &valueColumn();
  }
  else
  {
    ActiveColumn = &valueColumn();
    InactiveColumn = &charColumn();
  }

  // get coord of click and whether this click was closer to the end of the pos
  KBufferCoord C( activeColumn().magPosOfX(Point.x()), lineAt(Point.y()) );

  BufferCursor->gotoCCoord( C );
}


int KHexEdit::indexByPoint( const QPoint &Point ) const
{
  const KBufferColumn *C;
  if( charColumn().isVisible() && Point.x() >= charColumn().x() )
    C = &charColumn();
  else
    C = &valueColumn();

  KBufferCoord Coord( C->posOfX(Point.x()), lineAt(Point.y()) );

  return BufferLayout->indexAtCCoord( Coord );
}


void KHexEdit::showEvent( QShowEvent *e )
{
    KColumnsView::showEvent( e );
    BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() );
}


bool KHexEdit::eventFilter( QObject *O, QEvent *E )
{
  if( O == this || O == viewport() )
  {
    if( E->type() == QEvent::FocusIn )
    {
      startCursor();
    }
    else if( E->type() == QEvent::FocusOut )
    {
      stopCursor();
    }
  }

//   if( O == this && E->type() == QEvent::PaletteChange )
//   {
//     QColor old( viewport()->colorGroup().color(QColorGroup::Text) );
//
//     if( old != colorGroup().color(QColorGroup::Text) )
//     {
//       QColor c( colorGroup().color(QColorGroup::Text) );
//       doc->setMinimumWidth( -1 );
//       doc->setDefaultFormat( doc->formatCollection()->defaultFormat()->font(), c );
//       lastFormatted = doc->firstParagraph();
//       formatMore();
//       repaintChanged();
//     }
//   }

  return QScrollView::eventFilter( O, E );
}


void KHexEdit::blinkCursor()
{
  // skip the cursor drawing?
  if( CursorPaused || InEditMode )
    return;

  // switch the cursor state
  paintActiveCursor( !BlinkCursorVisible );
}


void KHexEdit::startCursor()
{
  CursorPaused = false;

  createCursorPixmaps();

  paintActiveCursor( true );
  paintInactiveCursor( true );

  CursorBlinkTimer->start( QApplication::cursorFlashTime()/2 );
}


void KHexEdit::unpauseCursor()
{
// std::cout << "unpauseCursor" <<std::endl;
  CursorPaused = false;

  updateCursor();
}


void KHexEdit::updateCursor()
{
  if( CursorBlinkTimer->isActive() )
  {
    createCursorPixmaps();

    paintActiveCursor( true );
    paintInactiveCursor( true );
  }
}

/*
void KHexEdit::unpauseCursorBlinking()
{
  CursorPaused = false;

  if( CursorBlinkTimer->isActive() && BufferCursor->isValid() )
  {
    createCursorPixmaps();
    drawCursor( true );
    paintInactiveCursor( true );
  }
}
*/
/*
void KHexEdit::stopCursorBlinking()
{
  CursorBlinkTimer->stop();

  // must Cursor be removed?
  if( BlinkCursorVisible )
    drawCursor( false );
  paintInactiveCursor( false );
  CursorPaused = true;
}


void KHexEdit::pauseCursorBlinking()
{
//   if( InEditMode )
//     goOutsideByte();
//   else
  {
    // must Cursor be removed?
    if( BlinkCursorVisible )
      drawCursor( false );
    paintInactiveCursor( false );
  }

  CursorPaused = true;
}
*/
void KHexEdit::stopCursor()
{
  CursorBlinkTimer->stop();

  pauseCursor();
}


void KHexEdit::pauseCursor( bool LeaveEdit )
{
// std::cout << "pauseCursor" <<std::endl;
  if( !InEditMode || LeaveEdit )
    paintActiveCursor( false );
  paintInactiveCursor( false );

  if( LeaveEdit )
    InEditMode = false;
  CursorPaused = true;
}


void KHexEdit::createCursorPixmaps()
{
  // create CursorPixmaps
  CursorPixmaps->setSize( activeColumn().byteWidth(), LineHeight );

  int Index = BufferCursor->validIndex();

  QPainter Paint;
  Paint.begin( &CursorPixmaps->offPixmap(), this );
  activeColumn().paintByte( &Paint, Index );
  Paint.end();

  Paint.begin( &CursorPixmaps->onPixmap(), this );
  activeColumn().paintCursor( &Paint, Index );
  Paint.end();

  // calculat the shape
  KPixelX CursorX;
  KPixelX CursorW;
  if( BufferCursor->isBehind() )
  {
    CursorX = QMAX( 0, CursorPixmaps->onPixmap().width()-InsertCursorWidth );
    CursorW = InsertCursorWidth;
  }
  else
  {
    CursorX = 0;
    CursorW = OverWrite ? -1 : InsertCursorWidth;
  }
  CursorPixmaps->setShape( CursorX, CursorW );
}


void KHexEdit::pointPainterToCursor( QPainter &Painter, const KBufferColumn &Column ) const
{
  int x = Column.xOfPos( BufferCursor->pos() ) - contentsX();
  int y = LineHeight * BufferCursor->line() - contentsY();

  Painter.begin( viewport() );
  Painter.translate( x, y );
}


void KHexEdit::paintActiveCursor( bool CursorOn )
{
  // any reason to skip the cursor drawing?
  if( !isUpdatesEnabled() || !viewport()->isUpdatesEnabled()
      || (CursorOn && !hasFocus() && !viewport()->hasFocus() && !InDnD )
      /*|| isReadOnly()*/ )
    return;

  if( InEditMode )
    paintEditedByte( CursorOn );
  else
  {
    QPainter Painter;
    pointPainterToCursor( Painter, activeColumn() );

    Painter.drawPixmap( CursorPixmaps->cursorX(), 0,
                        CursorOn?CursorPixmaps->onPixmap():CursorPixmaps->offPixmap(),
                        CursorPixmaps->cursorX(),0,CursorPixmaps->cursorW(),-1 );
    // store state
    BlinkCursorVisible = CursorOn;
  }
}


void KHexEdit::paintInactiveCursor( bool CursorOn )
{
  // any reason to skip the cursor drawing?
  if( !isUpdatesEnabled()
      || !viewport()->isUpdatesEnabled()
      || !inactiveColumn().isVisible()
      || (CursorOn && !hasFocus() && !viewport()->hasFocus() && !InDnD)  )
    return;

  int Index = BufferCursor->validIndex();

  QPainter Painter;
  pointPainterToCursor( Painter, inactiveColumn() );
  if( CursorOn )
  {
    KBufferColumn::KFrameStyle Style = BufferCursor->isBehind()?KBufferColumn::Right:
                                       (OverWrite||InEditMode)?KBufferColumn::Frame:KBufferColumn::Left;
    inactiveColumn().paintFramedByte( &Painter, Index, Style );
  }
  else
    inactiveColumn().paintByte( &Painter, Index );
}


void KHexEdit::paintEditedByte( bool Edited )
{
  // any reason to skip the cursor drawing?
  if( !isUpdatesEnabled() || !viewport()->isUpdatesEnabled() )
    return;

  int Index = BufferCursor->index();

  QPainter Painter;
  pointPainterToCursor( Painter, valueColumn() );
  if( Edited )
    valueColumn().paintEditedByte( &Painter, EditValue, ByteBuffer );
  else
    valueColumn().paintByte( &Painter, Index );
}


void KHexEdit::drawContents( QPainter *P, int cx, int cy, int cw, int ch )
{
  KColumnsView::drawContents( P, cx, cy, cw, ch );
  // TODO: update non blinking cursors. Should this perhaps be done in the buffercolumn?
  // Then it needs to know about inactive, insideByte and the like... well...
  // perhaps subclassing the buffer columns even more, to KCharColumn and KValueColumn?

  if( !CursorPaused && visibleLines(KPixelYs(cy,ch,false)).includes(BufferCursor->line()) )
    updateCursor();
}


void KHexEdit::updateColumn( KColumn &Column )
{
  //kdDebug(1501) << "updateColumn\n";
  if( Column.isVisible() )
    updateContents( Column.x(), 0, Column.width(), totalHeight() );
}


bool KHexEdit::handleByteEditKey( QKeyEvent *KeyEvent )
{
  bool KeyUsed = false;
  //
  switch( KeyEvent->key() )
  {
    case Key_Plus:
      incByte();
      KeyUsed = true;
      break;
    case Key_Minus:
      decByte();
      KeyUsed = true;
      break;
    case Key_Enter:
    case Key_Return:
    case Key_Space:
      goOutsideByte( !OverWrite );
      KeyUsed = true;
      break;
    case Key_Escape:
      EditValue = OldValue;
      syncEditedByte();
      goOutsideByte();
      KeyUsed = true;
      break;
    case Key_Backspace:
      if( EditValue > 0 )
      {
        valueColumn().removingFunction()( &EditValue );
        syncEditedByte();
        updateCursor(); //paintEditedByte( true );
      }
      else
        emit inputFailed();
      KeyUsed = true;
      break;
    default:
      if( KeyEvent->text().length() > 0
          && ( !(KeyEvent->state()&( ControlButton | AltButton | MetaButton )) )
          && ( !KeyEvent->ascii() || KeyEvent->ascii() >= 32 ) )
      {
        if( valueColumn().appendingFunction()(&EditValue,KeyEvent->ascii()) )
        {
          syncEditedByte();
          if( EditModeByInsert && valueColumn().digitsFilled(EditValue) )
            goOutsideByte( true );
          else
            updateCursor();
        }
        else
        {
          emit inputFailed();
        }
        KeyUsed = true;
      }
  }

  return KeyUsed;
}


void KHexEdit::keyPressEvent( QKeyEvent *KeyEvent )
{
  if( InEditMode && handleByteEditKey(KeyEvent) )
    return;
//  changeIntervalTimer->stop();
//  interval = 10;
  bool KeyUnknown = false;

  // handle selection
  BufferRanges->removeFurtherSelections();
  if( BufferRanges->isModified() )
    repaintChanged();

  bool clearUndoRedoInfo = true;
  bool ShiftPressed =  KeyEvent->state() & ShiftButton;
  bool ControlPressed = KeyEvent->state() & ControlButton;
  bool AltPressed = KeyEvent->state() & AltButton;

  // we only care for cursor keys and the like, won't hardcode any other keys
  // we also don't check whether the commands are allowed
  // as the commands are also available as API so the check has to be done
  // in each command anyway
  switch( KeyEvent->key() )
  {
    case Key_Left:
      moveCursor( ControlPressed ? MoveWordBackward : MoveBackward, ShiftPressed );
      break;
    case Key_Right:
      moveCursor( ControlPressed ? MoveWordForward : MoveForward, ShiftPressed );
      break;
    case Key_Up:
      moveCursor( ControlPressed ? MovePgUp : MoveUp, ShiftPressed );
      break;
    case Key_Down:
      moveCursor( ControlPressed ? MovePgDown : MoveDown, ShiftPressed );
      break;
    case Key_Home:
      moveCursor( ControlPressed ? MoveHome : MoveLineStart, ShiftPressed );
      break;
    case Key_End:
      moveCursor( ControlPressed ? MoveEnd : MoveLineEnd, ShiftPressed );
      break;
    case Key_Prior:
      moveCursor( MovePgUp, ShiftPressed );
      break;
    case Key_Next:
      moveCursor( MovePgDown, ShiftPressed );
      break;
    case Key_Return:
    case Key_Enter:
      goInsideByte();
//       if( isReadOnly() )
//         viewport()->setCursor( arrowCursor );

//       doKeyboardAction( ActionReturn );
//       emit returnPressed();

//       KeyEvent->ignore();
      break;
    case Key_Tab:
      if( ActiveColumn == &charColumn() )
      {
        // in last column we care about tab changes focus
        if( TabChangesFocus && !ShiftPressed )
        {
          // leave tab for parent widget
          KeyEvent->ignore();
          break;
        }
        if( valueColumn().isVisible() )
        {
          pauseCursor();
          ActiveColumn = &valueColumn();
          InactiveColumn = &charColumn();
          unpauseCursor();
        }
        break;
      }

      if( charColumn().isVisible() )
      {
        // in last column we care about tab changes focus
        if( TabChangesFocus && ShiftPressed )
        {
          // leave tab for parent widget
          KeyEvent->ignore();
          break;
        }
        pauseCursor( true );
        ActiveColumn = &charColumn();
        InactiveColumn = &valueColumn();
        unpauseCursor();
      }
      break;

    case Key_Delete:
      if( ShiftPressed )
        cut();
      else if( BufferRanges->hasSelection() )
        removeSelectedData();
      else
      {
        doKeyboardAction( ControlPressed ? ActionWordDelete : ActionDelete );
        clearUndoRedoInfo = false;
      }
      break;

    case Key_Insert:
      if( ShiftPressed )
        paste();
      else if( ControlPressed )
        copy();
      else
        setOverwriteMode( !OverWrite );
      break;

    case Key_Backspace:
      if( AltPressed )
      {
        if( ControlPressed )
          break;
        else if( ShiftPressed )
        {
//           redo();
          break;
        }
        else
        {
//           undo();
          break;
        }
      }
      else if( BufferRanges->hasSelection() )
      {
        removeSelectedData();
        break;
      }

      doKeyboardAction( ControlPressed ? ActionWordBackspace : ActionBackspace );
      clearUndoRedoInfo = false;
      break;
    case Key_F16: // "Copy" key on Sun keyboards
      copy();
      break;
    case Key_F18: // "Paste" key on Sun keyboards
      paste();
      break;
    case Key_F20: // "Cut" key on Sun keyboards
      cut();
      break;

    default:
      if( KeyEvent->text().length() > 0
          && ( !(KeyEvent->state()&( ControlButton | AltButton | MetaButton )) )
          && ( !KeyEvent->ascii() || KeyEvent->ascii() >= 32 ) )
      {
        if( isReadOnly() || !handleLetter(KeyEvent) )
          emit inputFailed();
      }
      else
        KeyUnknown = true;
  }

  emit cursorPositionChanged( BufferCursor->index() );

//   if( clearUndoRedoInfo )
//     clearUndoRedo();
//   changeIntervalTimer->start( 100, true );

  if( KeyUnknown )
    KeyEvent->ignore();
}


bool KHexEdit::handleLetter( QKeyEvent *KeyEvent )
{
  if( ActiveColumn == &charColumn() )
  {
    QByteArray D( 1 );
    if( !Codec->encode(&D[0],KeyEvent->text()[0]) )
      return false;
//         clearUndoRedoInfo = false;
    insert( D );
    ensureCursorVisible();    
  }
  else
  {
    if( BufferRanges->hasSelection() )
      return false;

    if( OverWrite )
    {
      int ValidIndex = BufferCursor->validIndex();
      if( ValidIndex == -1 || BufferCursor->isBehind() )
        return false;

      // check for plus/minus
      switch( KeyEvent->key() )
      {
        case Key_Plus:
          return incByte();
        case Key_Minus:
          return decByte();
      }

      OldValue = (unsigned char)DataBuffer->datum( ValidIndex );
    }

    EditValue = 0;
    bool KeyValid = valueColumn().appendingFunction()( &EditValue, KeyEvent->ascii() );
    if( !KeyValid )
      return false;

    pauseCursor();

    if( OverWrite )
    {
      InEditMode = true;
      EditModeByInsert = true;
    }
    else
    {
      int Index = BufferCursor->realIndex();
      int W = DataBuffer->insert( Index, (char*)&EditValue, 1 );
      if( W > 0 )
      {
        updateLength();
        BufferRanges->addChangedRange( KSection(Index+1,DataBuffer->size()-1) );
        BufferCursor->gotoRealIndex();
        InEditMode = true;
        EditModeByInsert = true;
        repaintChanged();
        ensureCursorVisible();
      }
      else
      {
        unpauseCursor();
        return false;
      }
    }

    syncEditedByte();

    unpauseCursor();

    emit bufferChanged();
  }
  return true;
}


void KHexEdit::moveCursor( KMoveAction Action, bool Select )
{
  pauseCursor( true );

  if( Select )
  {
    if( !BufferRanges->selectionStarted() )
      BufferRanges->setSelectionStart( BufferCursor->realIndex() );

    moveCursor( Action );
    BufferRanges->setSelectionEnd( BufferCursor->realIndex() );

    if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
    emit copyAvailable( BufferRanges->hasSelection() );
    emit selectionChanged();
  }
  else
  {
    moveCursor( Action );
    BufferRanges->removeSelection();

    if( BufferRanges->isModified() )
    {
      viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );

      if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
      emit copyAvailable( BufferRanges->hasSelection() );
      emit selectionChanged();
    }
  }

  repaintChanged();
  ensureCursorVisible();

  unpauseCursor();
}


// TODO: table of member functions, indexes by enum CursorAction (CharType would be stored at cursor)
void KHexEdit::moveCursor( KMoveAction Action )
{
  resetInputContext();
  switch( Action )
  {
    case MoveBackward:     BufferCursor->gotoPreviousByte(); break;
    case MoveWordBackward: {
                             int NewIndex = BufferCursor->realIndex();
                             NewIndex = DataBuffer->indexOfPreviousWordStart( NewIndex, KDataBuffer::Readable );
                             BufferCursor->gotoIndex( NewIndex );
                           }
                           break;
    case MoveForward:      BufferCursor->gotoNextByte();     break;
    case MoveWordForward:  {
                             int NewIndex = BufferCursor->realIndex();
                             NewIndex = DataBuffer->indexOfNextWordStart( NewIndex, KDataBuffer::Readable );
                             BufferCursor->gotoCIndex( NewIndex );
                           }
                           break;
    case MoveUp:           BufferCursor->gotoUp();             break;
    case MovePgUp:         BufferCursor->gotoPageUp();         break;
    case MoveDown:         BufferCursor->gotoDown();           break;
    case MovePgDown:       BufferCursor->gotoPageDown();       break;
    case MoveLineStart:    BufferCursor->gotoLineStart();      break;
    case MoveHome:         BufferCursor->gotoStart();          break;
    case MoveLineEnd:      BufferCursor->gotoLineEnd();        break;
    case MoveEnd:          BufferCursor->gotoEnd();            break;
  }
}


void KHexEdit::doKeyboardAction( KKeyboardAction Action )
{
  if( isReadOnly() )
    return;

  pauseCursor( true );

  switch( Action )
  {
    case ActionDelete:
//       if( BufferCursor->isBehind() )
//       {
//         if ( undoEnabled )
//         {
//           checkUndoRedoInfo( UndoRedoInfo::Delete );
//           if ( !undoRedoInfo.valid() )
//           {
//             undoRedoInfo.id = cursor->paragraph()->paragId();
//             undoRedoInfo.index = cursor->index();
//             undoRedoInfo.d->text = QString::null;
//           }
//           undoRedoInfo.d->text.insert( undoRedoInfo.d->text.length(), cursor->paragraph()->at( cursor->index() ), true );
//         }
//         cursor->remove();
//       }
//       else
//       {
//         clearUndoRedo();
//         doc->setSelectionStart( QTextDocument::Temp, *cursor );
//         if( Action == ActionWordDelete && !cursor->atParagEnd() )
//           Cursor->gotoNextWord();
//         else
//           Cursor->gotoNextLetter();
//
//         doc->setSelectionEnd( QTextDocument::Temp, *cursor );
//         removeSelectedData( QTextDocument::Temp );
//       }
      if( !OverWrite )
      {
        int Index = BufferCursor->realIndex();
        if( Index < BufferLayout->length() )
        {
          removeData( KSection(Index,1,false) );
          if( Index == BufferLayout->length() )
            BufferCursor->gotoEnd();
        }
      }
      break;

    case ActionWordDelete: // kills data until the start of the next word
      if( !OverWrite )
      {
        int Index = BufferCursor->realIndex();
        if( Index < BufferLayout->length() )
        {
          int End = DataBuffer->indexOfBeforeNextWordStart( Index );
          removeData( KSection(Index,End) );
          if( Index == BufferLayout->length() )
            BufferCursor->gotoEnd();
        }
      }
      break;

    case ActionBackspace:
      if( OverWrite )
        BufferCursor->gotoPreviousByte();
      else
      {
        int DeleteIndex = BufferCursor->realIndex() - 1;
        if( DeleteIndex >= 0 )
        {
          removeData( KSection(DeleteIndex,1,false) );
          if( DeleteIndex == BufferLayout->length() )
            BufferCursor->gotoEnd();
          else
            BufferCursor->gotoPreviousByte();
        }
      }
      break;

//       if( Action == ActionBackspace && !cursor->atParagStart() )
//       {
//         if( undoEnabled )
//         {
//           checkUndoRedoInfo( UndoRedoInfo::Delete );
//           if( !undoRedoInfo.valid() )
//           {
//             undoRedoInfo.id = cursor->paragraph()->paragId();
//             undoRedoInfo.index = cursor->index();
//             undoRedoInfo.d->text = QString::null;
//           }
//         }
//         cursor->gotoPreviousLetter();
//         if ( undoEnabled )
//         {
//           undoRedoInfo.d->text.insert( 0, cursor->paragraph()->at( cursor->index() ), true );
//           undoRedoInfo.index = cursor->index();
//         }
//         cursor->remove();
//         lastFormatted = cursor->paragraph();
//       }
//       else if( cursor->paragraph()->prev()
//                || (Action == ActionWordBackspace && !cursor->atParagStart()) )
//       {
//         clearUndoRedo();
//         doc->setSelectionStart( QTextDocument::Temp, *cursor );
//
//         if ( Action == ActionWordBackspace && !cursor->atParagStart() )
//           cursor->gotoPreviousWord();
//         else
//           cursor->gotoPreviousLetter();
//
//         doc->setSelectionEnd( QTextDocument::Temp, *cursor );
//         removeSelectedData( QTextDocument::Temp );
//       }
    case ActionWordBackspace:
      {
        int LeftIndex = BufferCursor->realIndex() - 1;
        if( LeftIndex >= 0 )
        {
          int WordStart = DataBuffer->indexOfPreviousWordStart( LeftIndex );
          if( !OverWrite )
            removeData( KSection(WordStart,LeftIndex) );
          if( WordStart == BufferLayout->length() )
            BufferCursor->gotoEnd();
          else
            BufferCursor->gotoIndex(WordStart);
        }
      }
  }

  repaintChanged();
  ensureCursorVisible();

  unpauseCursor();

  emit cursorPositionChanged( BufferCursor->index() );
  emit bufferChanged();
}


void KHexEdit::repaintChanged()
{
  if( !isUpdatesEnabled() || !viewport()->isUpdatesEnabled() || !BufferRanges->isModified() )
    return;

  // TODO: we do this only to let the scrollview handle new or removed lines. overlaps with repaintRange
  resizeContents( totalWidth(), totalHeight() );

  KPixelX cx = contentsX();
  KPixelY cy = contentsY();
  KPixelX cw = visibleWidth();
  KPixelY ch = visibleHeight();

  // calculate affected lines/indizes
  KSection VisibleLines = visibleLines( KPixelYs(cy,ch,false) );
  KSection FullPositions( 0, BufferLayout->noOfBytesPerLine()-1 );
  KCoordRange VisibleRange( FullPositions, VisibleLines );
//   std::cout << "repaintChanged->"<<FirstIndex<<":"<<FirstLine<<","<<LastIndex<<":"<<LastLine<<std::endl;

  // collect affected buffer columns
  QPtrList<KBufferColumn> RepaintColumns;

  KBufferColumn *C = ValueColumn;
  while( true )
  {
    if( C->isVisible() && C->overlaps(cx,cx+cw-1) )
    {
      RepaintColumns.append( C );
      C->preparePainting( cx, cw );
    }

    if( C == CharColumn )
      break;
    C = CharColumn;
  }

  if( RepaintColumns.count() > 0 )
  {
    KCoordRange ChangedRange;
    // as there might be multiple selections on this line redo until no more is changed
    while( hasChanged(VisibleRange,&ChangedRange) )
    {
//       std::cout << "  changed->"<<FirstChangedIndex<<","<<LastChangedIndex<<std::endl;

      // only one line?
      if( ChangedRange.start().line() == ChangedRange.end().line() )
        for( KBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
          paintLine( C, ChangedRange.start().line(),
                     KSection(ChangedRange.start().pos(),ChangedRange.end().pos()) );
      //
      else
      {
        // first line
        for( KBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
          paintLine( C, ChangedRange.start().line(),
                     KSection(ChangedRange.start().pos(),FullPositions.end()) );

        // at least one full line?
        for( int l = ChangedRange.start().line()+1; l < ChangedRange.end().line(); ++l )
          for( KBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
            paintLine( C, l, FullPositions );

        // last line
        for( KBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
          paintLine( C, ChangedRange.end().line(),
                     KSection(FullPositions.start(),ChangedRange.end().pos()) );
      }

      // continue the search at the overnext index
      VisibleRange.setStart( ChangedRange.end()+2 );
      if( !VisibleRange.isValid() )
        break;
    }
  }


  // Paint possible removed bytes at the end of the last line
  // Paint new/removed trailing lines
//   drawContents( P, cx, cy, cw, ch );
  // Paint empty rects
//  paintEmptyArea( P, cx, cy, cw, ch );
//   BufferLayout->noOfLines()

  BufferRanges->resetChangedRanges();
}


void KHexEdit::paintLine( KBufferColumn *C, int Line, KSection Positions )
{
  Positions.restrictTo( C->visiblePositions() );

  // nothing to paint?
  if( !Positions.isValid() )
    return;
//   std::cout << "  paintLine->"<<Line<< ":"<<FirstPos<<","<<LastPos<<std::endl;

  // calculating pixel values
  KPixelXs XPixels = C->wideXPixelsOfPos( Positions );

  KPixelY cy = Line * LineHeight;

  // to avoid flickers we first paint to the linebuffer
  QPainter Paint;
  Paint.begin( &LineBuffer, this );

  Paint.translate( C->x(), 0 );
  C->paintPositions( &Paint, Line, Positions );
  Paint.translate( -C->x(), 0 );

  if( HorizontalGrid && XPixels.start() < TotalWidth )
    Paint.drawLine( XPixels.start(), LineHeight-1, XPixels.width(), LineHeight-1 );  // TODO: use a additional TotalHeight?

  Paint.end();
  // copy to screen
  bitBlt( viewport(), XPixels.start() - contentsX(), cy - contentsY(),
          &LineBuffer, XPixels.start(), 0, XPixels.width(), LineHeight );
}


bool KHexEdit::hasChanged( const KCoordRange &VisibleRange, KCoordRange *ChangedRange ) const
{
  if( !BufferRanges->overlapsChanges(VisibleRange,ChangedRange) )
    return false;

  ChangedRange->restrictTo( VisibleRange );
  return true;
}


void KHexEdit::ensureCursorVisible()
{
//   // Not visible or the user is draging the window, so don't position to caret yet
//   if ( !isVisible() || isHorizontalSliderPressed() || isVerticalSliderPressed() )
//   {
//     d->ensureCursorVisibleInShowEvent = true;
//     return;
//   }

  KPixelX x = activeColumn().x() + activeColumn().xOfPos( BufferCursor->pos() );
  KPixelY y = LineHeight * BufferCursor->line() + LineHeight/2;
  int xMargin = 1; // TODO: reasoning of the values 1 and 2 (below)
  int yMargin = LineHeight/2 + 2;
  ensureVisible( x, y, xMargin, yMargin );
}



void KHexEdit::contentsMousePressEvent( QMouseEvent *e )
{
//   clearUndoRedo();
  pauseCursor( true );

  // care about a left button press?
  if( e->button() == LeftButton )
  {
    MousePressed = true;

    // select whole line?
    if( TrippleClickTimer->isActive()
        && (e->globalPos()-DoubleClickPoint).manhattanLength() < QApplication::startDragDistance() )
    {
      BufferRanges->setSelectionStart( BufferLayout->indexAtLineStart(DoubleClickLine) );
      BufferCursor->gotoLineEnd();
      BufferRanges->setSelectionEnd( BufferCursor->realIndex() );
      repaintChanged();

      unpauseCursor();
      return;
    }

    QPoint MousePoint = e->pos();
    placeCursor( MousePoint );
    ensureCursorVisible();

    // start of a drag perhaps?
    if( BufferRanges->selectionIncludes(BufferCursor->index()) )
    {
      DragStartPossible = true;
      DragStartTimer->start( QApplication::startDragTime(), true );
      DragStartPoint = MousePoint;

      unpauseCursor();
      return;
    }

    int RealIndex = BufferCursor->realIndex();
    if( BufferRanges->selectionStarted() )
    {
      if( e->state() & ShiftButton )
        BufferRanges->setSelectionEnd( RealIndex );
      else
      {
        BufferRanges->removeSelection();
        BufferRanges->setSelectionStart( RealIndex );
      }
    }
    else // start of a new selection possible
    {
      BufferRanges->setSelectionStart( RealIndex );

      if( !isReadOnly() && (e->state()&ShiftButton) ) // TODO: why only for readwrite?
        BufferRanges->setSelectionEnd( RealIndex );
    }

    BufferRanges->removeFurtherSelections();
  }
  else if( e->button() == MidButton )
    BufferRanges->removeSelection();

  if( BufferRanges->isModified() )
  {
    repaintChanged();
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
  }

  unpauseCursor();
}


void KHexEdit::contentsMouseMoveEvent( QMouseEvent *e )
{
  if( MousePressed )
  {
    if( DragStartPossible )
    {
      DragStartTimer->stop();
      // moved enough for a drag?
      if( (e->pos()-DragStartPoint).manhattanLength() > QApplication::startDragDistance() )
        startDrag();
      if( !isReadOnly() )
        viewport()->setCursor( ibeamCursor );
      return;
    }
    // selecting
    QPoint MousePoint = e->pos();
    handleMouseMove( MousePoint );
  }
  else if( !isReadOnly() )
  {
    // visual feedback for possible dragging
    bool InSelection = BufferRanges->hasSelection() && BufferRanges->selectionIncludes( indexByPoint(e->pos()) );
    viewport()->setCursor( InSelection?arrowCursor:ibeamCursor );
  }
}


void KHexEdit::contentsMouseReleaseEvent( QMouseEvent *e )
{
  // this is not the release of a doubleclick so we need to process it?
  if( !InDoubleClick )
  {
    int Line = lineAt( e->pos().y() );
    int Pos = activeColumn().posOfX( e->pos().x() ); // TODO: can we be sure here about the active column?
    int Index = BufferLayout->indexAtCCoord( KBufferCoord(Pos,Line) ); // TODO: can this be another index than the one of the cursor???
    emit clicked( Index );
  }

  if( MousePressed )
  {
    MousePressed = false;

    if( ScrollTimer->isActive() )
      ScrollTimer->stop();

    // was only click inside selection, nothing dragged?
    if( DragStartPossible )
    {
      selectAll( false );
      DragStartTimer->stop();
      DragStartPossible = false;

      unpauseCursor();
    }
    // was end of selection operation?
    else if( BufferRanges->hasSelection() )
    {
      if( QApplication::clipboard()->supportsSelection() )
      {
        ClipboardMode = QClipboard::Selection;
        disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);

        copy();

        connect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardChanged()) );
        ClipboardMode = QClipboard::Clipboard;
      }
    }
  }
  // middle mouse button paste?
  else if( e->button() == MidButton && !isReadOnly() )
  {
    pauseCursor();

    placeCursor( e->pos() );

    // replace no selection?
    if( BufferRanges->hasSelection() && !BufferRanges->selectionIncludes(BufferCursor->index()) )
      BufferRanges->removeSelection();

    ClipboardMode = QClipboard::Selection;
    paste();
    ClipboardMode = QClipboard::Clipboard;

    // ensure selection changes to be drawn TODO: create a insert/pasteAtCursor that leaves out drawing
    repaintChanged();

    ensureCursorVisible();
    unpauseCursor();
  }

  emit cursorPositionChanged( BufferCursor->index() );

  InDoubleClick = false;

  if( BufferRanges->selectionJustStarted() )
    BufferRanges->removeSelection();

  if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
  emit copyAvailable( BufferRanges->hasSelection() );
  emit selectionChanged();
}


// gets called after press and release instead of a plain press event (?)
void KHexEdit::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
  // we are only interested in LMB doubleclicks
  if( e->button() != Qt::LeftButton )
  {
    e->ignore();
    return;
  }

  DoubleClickLine = BufferCursor->line();

  int Index = BufferCursor->validIndex();

  if( ActiveColumn == &charColumn() )
  {
    selectWord( Index );
    
    // as we already have a doubleclick maybe it is a tripple click
    TrippleClickTimer->start( qApp->doubleClickInterval(), true );
    DoubleClickPoint = e->globalPos();
  }
  else
  {
    goInsideByte();
  }


  InDoubleClick = true; //
  MousePressed = true;

  emit doubleClicked( Index );
}


void KHexEdit::autoScrollTimerDone()
{
  if( MousePressed )
    handleMouseMove( viewportToContents(viewport()->mapFromGlobal( QCursor::pos() )) );
}


void KHexEdit::handleMouseMove( const QPoint& Point ) // handles the move of the mouse with pressed buttons
{
  // no scrolltimer and outside of viewport?
  if( !ScrollTimer->isActive() && Point.y() < contentsY() || Point.y() > contentsY() + visibleHeight() )
    ScrollTimer->start( DefaultScrollTimerPeriod, false );
  // scrolltimer but inside of viewport?
  else if( ScrollTimer->isActive() && Point.y() >= contentsY() && Point.y() <= contentsY() + visibleHeight() )
    ScrollTimer->stop();

  pauseCursor();

  placeCursor( Point );
  ensureCursorVisible();

  // do wordwise selection?
  if( InDoubleClick && BufferRanges->hasFirstWordSelection() ) 
  {
    int NewIndex = BufferCursor->realIndex();
    KSection FirstWordSelection = BufferRanges->firstWordSelection();
    // are we before the selection?
    if( NewIndex < FirstWordSelection.start() )
    {
      BufferRanges->ensureWordSelectionForward( false );
      NewIndex = DataBuffer->indexOfLeftWordSelect( NewIndex );
    } 
    // or behind?
    else if( NewIndex > FirstWordSelection.end() )
    {
      BufferRanges->ensureWordSelectionForward( true );
      NewIndex = DataBuffer->indexOfRightWordSelect( NewIndex );
    }
    // or inside?
    else
    {   
      BufferRanges->ensureWordSelectionForward( true );
      NewIndex = FirstWordSelection.end()+1;
    }
  
    BufferCursor->gotoIndex( NewIndex );    
  }

  if( BufferRanges->selectionStarted() )
    BufferRanges->setSelectionEnd( BufferCursor->realIndex() );

  repaintChanged();

  unpauseCursor();
}


void KHexEdit::startDrag()
{
  // reset states
  MousePressed = false;
  InDoubleClick = false;
  DragStartPossible = false;

  // create data
  QDragObject *Drag = dragObject( viewport() );
  if( !Drag )
    return;

  // will we only copy the data?
  if( isReadOnly() || OverWrite )
    Drag->dragCopy();
  // or is this left to the user and he choose to move?
  else if( Drag->drag() )
    // Not inside this widget itself?
    if( QDragObject::target() != this && QDragObject::target() != viewport() )
      removeSelectedData();
}


void KHexEdit::contentsDragEnterEvent( QDragEnterEvent *e )
{
  // interesting for this widget?
  if( isReadOnly() || !KBufferDrag::canDecode(e) )
  {
    e->ignore();
    return;
  }
  
  e->acceptAction();
  InDnD = true;
}


void KHexEdit::contentsDragMoveEvent( QDragMoveEvent *e )
{
  // is this content still interesting for us? 
  if( isReadOnly() || !KBufferDrag::canDecode(e) )
  {
    e->ignore();
    return;
  }
  
  // let text cursor follow mouse
  pauseCursor( true );
  placeCursor( e->pos() );
  unpauseCursor();
  
  e->acceptAction();
}


void KHexEdit::contentsDragLeaveEvent( QDragLeaveEvent * )
{
  // bye... and thanks for all the cursor movement...
  InDnD = false;
}



void KHexEdit::contentsDropEvent( QDropEvent *e )
{
  // after drag enter and move check one more time
  if( isReadOnly() )
    return;

  // leave state
  InDnD = false;
  e->acceptAction();
  
  if( !KBufferDrag::canDecode(e) ) //TODO: why do we acept the action still?
    return;
   
  // is this an internal dnd?
  if( e->source() == this || e->source() == viewport() )
    handleInternalDrag( e );
  else
  {
   //BufferRanges->removeSelection(); 
    pasteFromSource( e ); 
  }
  
  // emit appropriate signals.
  emit selectionChanged();
  emit cursorPositionChanged( BufferCursor->index() );
}


void KHexEdit::handleInternalDrag( QDropEvent *e )
{
  // stop ui
  pauseCursor();
    
  // get drag origin
  KSection Selection = BufferRanges->selection();
  int InsertIndex = BufferCursor->realIndex();
    
  // is this a move?    
  if( e->action() == QDropEvent::Move )
  {
    // ignore the copy hold in the event but only move 
    int NewIndex = DataBuffer->move( InsertIndex, Selection );
    if( NewIndex != Selection.start() )
    {
      BufferCursor->gotoCIndex( NewIndex+Selection.width() );
      BufferRanges->addChangedRange( KSection(QMIN(InsertIndex,Selection.start()), QMAX(InsertIndex,Selection.end())) );
    }
  }
  // is a copy
  else
  {
    // get data  
    QByteArray Data;
    if( KBufferDrag::decode(e,Data) && !Data.isEmpty() )
    {    
      if( OverWrite )
      {
        KSection Section( InsertIndex, Data.size(), false );
        Section.restrictEndTo( BufferLayout->length()-1 );
        if( Section.isValid() && !BufferCursor->isBehind() )
        {
          int NoOfReplaced = DataBuffer->replace( Section, Data.data(), Section.width() );
          BufferCursor->gotoNextByte( NoOfReplaced );
          BufferRanges->addChangedRange( Section );
        }
      }
      else
      {
        int NoOfInserted = DataBuffer->insert( InsertIndex, Data.data(), Data.size() );
        updateLength();
        if( NoOfInserted > 0 )
        {
          BufferCursor->gotoCIndex( InsertIndex + NoOfInserted );
          BufferRanges->addChangedRange( KSection(InsertIndex,DataBuffer->size()-1) );
        }
      }    
    }
  }
  BufferRanges->removeSelection();
  repaintChanged();
  ensureCursorVisible();
         
  // open ui
  unpauseCursor();    
}


void KHexEdit::contentsWheelEvent( QWheelEvent *e )
{
  if( isReadOnly() )
  {
    if( e->state() & ControlButton )
    {
      if( e->delta() > 0 )
        zoomOut();
      else if( e->delta() < 0 )
        zoomIn();
      return;
    }
  }
  QScrollView::contentsWheelEvent( e );
}


#if 0
void KHexEdit::contentsContextMenuEvent( QContextMenuEvent *e )
{
//   clearUndoRedo();
  MousePressed = false;

  e->accept();

  QPopupMenu *PopupMenu = createPopupMenu( e->pos() );
  if( !PopupMenu )
    PopupMenu = createPopupMenu();
  if( !PopupMenu )
    return;
  int r = PopupMenu->exec( e->globalPos() );
  delete PopupMenu;

  if ( r == d->id[ IdClear ] )
    clear();
  else if ( r == d->id[ IdSelectAll ] )
  {
    selectAll();
    // if the clipboard support selections, put the newly selected text into the clipboard
    if( QApplication::clipboard()->supportsSelection() )
    {
      ClipboardMode = QClipboard::Selection;
      disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);

      copy();

      connect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardChanged()) );
      ClipboardMode = QClipboard::Clipboard;
    }
  }
  else if( r == d->id[IdUndo] )
    undo();
  else if( r == d->id[IdRedo] )
    redo();
  else if( r == d->id[IdCut] )
    cut();
  else if( r == d->id[IdCopy] )
    copy();
  else if( r == d->id[IdPaste] )
    paste();
}
#endif

#include "khexedit.moc"
