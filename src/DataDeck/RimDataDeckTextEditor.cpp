#include "RimDataDeckTextEditor.h"
#include "DataFileSyntaxHighlighter.h"
#include "RimDataDeck.h"

#include <QPainter>
#include <QTextBlock>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeckTextEditor::RimDataDeckTextEditor( QWidget* parent )
    : QPlainTextEdit( parent )
    , m_dataDeck( nullptr )
    , m_syntaxHighlighter( nullptr )
    , m_lineNumberArea( nullptr )
{
    // Setup line number area
    m_lineNumberArea = new LineNumberArea( this );

    connect( this, &RimDataDeckTextEditor::blockCountChanged, this, &RimDataDeckTextEditor::updateLineNumberAreaWidth );
    connect( this, &RimDataDeckTextEditor::updateRequest, this, &RimDataDeckTextEditor::updateLineNumberArea );
    connect( this, &RimDataDeckTextEditor::cursorPositionChanged, this, &RimDataDeckTextEditor::highlightCurrentLine );

    // Setup syntax highlighting
    m_syntaxHighlighter = new DataFileSyntaxHighlighter( document() );

    // Setup font
    QFont font;
    font.setFamily( "Cascadia Mono" );
    font.setStyleHint( QFont::Monospace );
    font.setPointSize( 10 );
    setFont( font );

    // Tab width (4 spaces)
    QFontMetrics metrics( font );
    setTabStopDistance( 4 * metrics.horizontalAdvance( ' ' ) );

    // Update line number area and margins after font is set
    updateLineNumberAreaWidth( 0 );
    highlightCurrentLine();

    // Connect modification signal
    connect( document(), &QTextDocument::modificationChanged, this, &RimDataDeckTextEditor::modificationChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeckTextEditor::~RimDataDeckTextEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::setDataDeck( RimDataDeck* dataDeck )
{
    m_dataDeck = dataDeck;
    if ( m_dataDeck )
    {
        loadFromDeck();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::loadFromDeck()
{
    if ( !m_dataDeck )
    {
        setPlainText( "" );
        return;
    }

    // Block signals to avoid triggering modification
    blockSignals( true );

    // Load text from deck
    QString text = m_dataDeck->serializeToText();
    setPlainText( text );

    // Mark as unmodified
    document()->setModified( false );

    blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataDeckTextEditor::hasUnsavedChanges() const
{
    return document()->isModified();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDataDeckTextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max    = qMax( 1, blockCount() );
    while ( max >= 10 )
    {
        max /= 10;
        ++digits;
    }

    int space = 10 + fontMetrics().horizontalAdvance( QLatin1Char( '9' ) ) * digits;

    return space;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::updateLineNumberAreaWidth( int /* newBlockCount */ )
{
    int width = lineNumberAreaWidth();
    setViewportMargins( width, 0, 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::updateLineNumberArea( const QRect& rect, int dy )
{
    if ( dy )
        m_lineNumberArea->scroll( 0, dy );
    else
        m_lineNumberArea->update( 0, rect.y(), m_lineNumberArea->width(), rect.height() );

    if ( rect.contains( viewport()->rect() ) ) updateLineNumberAreaWidth( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::resizeEvent( QResizeEvent* e )
{
    QPlainTextEdit::resizeEvent( e );

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry( QRect( cr.left(), cr.top(), lineNumberAreaWidth(), cr.height() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if ( !isReadOnly() )
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor( Qt::yellow ).lighter( 160 );

        selection.format.setBackground( lineColor );
        selection.format.setProperty( QTextFormat::FullWidthSelection, true );
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append( selection );
    }

    setExtraSelections( extraSelections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::lineNumberAreaPaintEvent( QPaintEvent* event )
{
    QPainter painter( m_lineNumberArea );
    painter.fillRect( event->rect(), palette().color( QPalette::Base ) );

    QTextBlock block       = firstVisibleBlock();
    int        blockNumber = block.blockNumber();
    int        top         = qRound( blockBoundingGeometry( block ).translated( contentOffset() ).top() );
    int        bottom      = top + qRound( blockBoundingRect( block ).height() );

    while ( block.isValid() && top <= event->rect().bottom() )
    {
        if ( block.isVisible() && bottom >= event->rect().top() )
        {
            QString number = QString::number( blockNumber + 1 );
            painter.setPen( Qt::gray );
            painter.drawText( 0, top, m_lineNumberArea->width() - 8, fontMetrics().height(), Qt::AlignRight, number );
        }

        block  = block.next();
        top    = bottom;
        bottom = top + qRound( blockBoundingRect( block ).height() );
        ++blockNumber;
    }
}
