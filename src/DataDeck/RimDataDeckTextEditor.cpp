#include "RimDataDeckTextEditor.h"
#include "DataFileSyntaxHighlighter.h"
#include "DataFileCompleter.h"
#include "KeywordHelpWidget.h"
#include "RimDataDeck.h"

#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTimer>
#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeckTextEditor::RimDataDeckTextEditor( QWidget* parent )
    : QPlainTextEdit( parent )
    , m_dataDeck( nullptr )
    , m_syntaxHighlighter( nullptr )
    , m_completer( nullptr )
    , m_helpWidget( nullptr )
    , m_lineNumberArea( nullptr )
    , m_helpUpdateTimer( nullptr )
{
    // Setup line number area
    m_lineNumberArea = new LineNumberArea( this );

    connect( this, &RimDataDeckTextEditor::blockCountChanged, this, &RimDataDeckTextEditor::updateLineNumberAreaWidth );
    connect( this, &RimDataDeckTextEditor::updateRequest, this, &RimDataDeckTextEditor::updateLineNumberArea );
    connect( this, &RimDataDeckTextEditor::cursorPositionChanged, this, &RimDataDeckTextEditor::highlightCurrentLine );
    connect( this, &RimDataDeckTextEditor::cursorPositionChanged, this, &RimDataDeckTextEditor::onCursorPositionChanged );

    // Setup syntax highlighting
    m_syntaxHighlighter = new DataFileSyntaxHighlighter( document() );

    // Setup completer
    setupCompleter();

    // Setup help update timer
    m_helpUpdateTimer = new QTimer( this );
    m_helpUpdateTimer->setSingleShot( true );
    m_helpUpdateTimer->setInterval( 500 ); // 500ms delay
    connect( m_helpUpdateTimer, &QTimer::timeout, this, &RimDataDeckTextEditor::updateKeywordHelp );

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

        QColor lineColor = palette().color(QPalette::Base).lighter(102); // Very subtle highlight close to background

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::setKeywordHelpWidget( KeywordHelpWidget* helpWidget )
{
    m_helpWidget = helpWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::keyPressEvent( QKeyEvent* event )
{
    if ( m_completer && m_completer->popup()->isVisible() )
    {
        // Handle completer popup keys
        switch ( event->key() )
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                event->ignore();
                return;
            default:
                break;
        }
    }

    bool isShortcut = ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Space);
    
    // Always process the key event first (except for the shortcut)
    if ( !isShortcut )
    {
        QPlainTextEdit::keyPressEvent( event );
    }

    // Early return if no completer
    if ( !m_completer )
        return;


    const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    
    QString completionPrefix = textUnderCursor();

    // Hide completer for certain conditions but don't return early for normal typing
    if ( !isShortcut && (hasModifier || event->text().isEmpty()) )
    {
        m_completer->popup()->hide();
        return;
    }
    
    // For end-of-word characters, hide popup but allow normal processing
    if ( !isShortcut && !event->text().isEmpty() && eow.contains(event->text().right(1)) )
    {
        m_completer->popup()->hide();
        return;
    }

    // Trigger completion on shortcut or when we have a valid prefix
    if ( isShortcut || completionPrefix.length() >= 1 )
    {
        if ( completionPrefix != m_completer->completionPrefix() )
        {
            m_completer->updateCompletions( textCursor() );
            m_completer->setCompletionPrefix( completionPrefix );
            m_completer->popup()->setCurrentIndex( m_completer->completionModel()->index(0, 0) );
        }

        QRect cr = cursorRect();
        cr.setWidth( m_completer->popup()->sizeHintForColumn(0)
                     + m_completer->popup()->verticalScrollBar()->sizeHint().width() );
        m_completer->complete( cr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::onCursorPositionChanged()
{
    // Start/restart the help update timer
    if ( m_helpUpdateTimer )
    {
        m_helpUpdateTimer->start();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::updateKeywordHelp()
{
    if ( !m_helpWidget )
        return;

    // Get the keyword under cursor or at line start
    QTextCursor cursor = textCursor();
    
    // Check if we're on a keyword line
    QTextCursor lineCursor = cursor;
    lineCursor.movePosition( QTextCursor::StartOfLine );
    QString lineText = lineCursor.block().text();
    
    static QRegularExpression keywordPattern( "^\\s*([A-Z][_A-Z0-9]*)\\b" );
    QRegularExpressionMatch match = keywordPattern.match( lineText );
    
    if ( match.hasMatch() )
    {
        QString keyword = match.captured( 1 );
        
        // Get current section context
        QString currentSection;
        if ( m_completer )
        {
            currentSection = m_completer->getCurrentSection( cursor );
        }
        
        m_helpWidget->showKeywordHelp( keyword, currentSection );
    }
    else
    {
        m_helpWidget->clearHelp();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::setupCompleter()
{
    m_completer = new DataFileCompleter( this );
    m_completer->setWidget( this );
    
    QObject::connect( m_completer, QOverload<const QString&>::of(&QCompleter::activated),
                     this, &RimDataDeckTextEditor::insertCompletion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeckTextEditor::insertCompletion( const QString& completion )
{
    if ( m_completer->widget() != this )
        return;
        
    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition( QTextCursor::Left );
    tc.movePosition( QTextCursor::EndOfWord );
    tc.insertText( completion.right( extra ) );
    setTextCursor( tc );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataDeckTextEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select( QTextCursor::WordUnderCursor );
    return tc.selectedText();
}
