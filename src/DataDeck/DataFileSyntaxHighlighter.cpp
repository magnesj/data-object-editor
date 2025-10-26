#include "DataFileSyntaxHighlighter.h"
#include "KeywordDatabase.h"

#include <QTextBlock>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
DataFileSyntaxHighlighter::DataFileSyntaxHighlighter( QTextDocument* parent )
    : QSyntaxHighlighter( parent )
    , m_keywordDatabase( KeywordDatabase::instance() )
{
    HighlightingRule rule;

    // Comments - must be first to take precedence
    m_commentFormat.setForeground( QColor( 106, 153, 85 ) ); // Green like in example
    m_commentFormat.setFontItalic( true );

    // Section keywords (RUNSPEC, GRID, PROPS, etc.)
    m_sectionKeywordFormat.setForeground( QColor( 86, 156, 214 ) ); // Blue like in example
    m_sectionKeywordFormat.setFontWeight( QFont::Bold );

    // Valid keywords
    m_keywordFormat.setForeground( QColor( 215, 186, 125 ) ); // Orange/yellow like in example
    m_keywordFormat.setFontWeight( QFont::Bold );

    // Invalid keywords (unknown or wrong context)
    m_invalidKeywordFormat.setForeground( QColor( 255, 100, 100 ) ); // Red
    m_invalidKeywordFormat.setFontWeight( QFont::Bold );

    // Context invalid (valid keyword in wrong section)
    m_contextInvalidFormat.setForeground( QColor( 255, 165, 0 ) ); // Orange
    m_contextInvalidFormat.setFontWeight( QFont::Bold );

    // INCLUDE keyword (important)
    m_includeFormat.setForeground( QColor( 197, 134, 192 ) ); // Purple/magenta like in example
    m_includeFormat.setFontWeight( QFont::Bold );
    rule.pattern = QRegularExpression( "^INCLUDE\\b" );
    rule.format  = m_includeFormat;
    m_rules.append( rule );

    // Include file paths (setup format, will be applied manually)
    m_includePathFormat.setForeground( QColor( 214, 157, 133 ) ); // Light orange/peach
    m_includePathFormat.setFontWeight( QFont::Normal );
    m_includePathFormat.setUnderlineStyle( QTextCharFormat::SingleUnderline );

    // Numbers (including scientific notation)
    m_numberFormat.setForeground( QColor( 181, 206, 168 ) ); // Dark yellow
    // Match: 123, 123.456, 1.23E+10, .5, etc.
    rule.pattern = QRegularExpression( "\\b[0-9]+\\.?[0-9]*([eE][+-]?[0-9]+)?\\b|\\b\\.[0-9]+([eE][+-]?[0-9]+)?\\b" );
    rule.format  = m_numberFormat;
    m_rules.append( rule );

    // Strings (single quoted)
    m_stringFormat.setForeground( QColor( 206, 145, 120 ) ); // Orange/salmon like strings in example
    rule.pattern = QRegularExpression( "'[^']*'" );
    rule.format  = m_stringFormat;
    m_rules.append( rule );

    // Variables <VARIABLE>
    QTextCharFormat variableFormat;
    variableFormat.setForeground( QColor( 220, 220, 170 ) ); // Light yellow like method calls in example
    rule.pattern = QRegularExpression( "<[A-Z][_A-Z0-9]*>" );
    rule.format  = variableFormat;
    m_rules.append( rule );

    // Parameters $PARAM
    QTextCharFormat paramFormat;
    paramFormat.setForeground( QColor( 0, 128, 128 ) ); // Teal
    rule.pattern = QRegularExpression( "\\$[A-Z][_A-Z0-9]*\\b" );
    rule.format  = paramFormat;
    m_rules.append( rule );

    // Delimiters
    m_delimiterFormat.setForeground( Qt::gray );
    m_delimiterFormat.setFontWeight( QFont::Bold );
    rule.pattern = QRegularExpression( "/" );
    rule.format  = m_delimiterFormat;
    m_rules.append( rule );

    // Initialize keyword sets
    initializeKeywordSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataFileSyntaxHighlighter::highlightBlock( const QString& text )
{
    // Check if line is a comment
    if ( text.trimmed().startsWith( "--" ) )
    {
        setFormat( 0, text.length(), m_commentFormat );
        return;
    }

    // Apply non-keyword rules first
    for ( const HighlightingRule& rule : m_rules )
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch( text );
        while ( matchIterator.hasNext() )
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat( match.capturedStart(), match.capturedLength(), rule.format );
        }
    }

    // Special handling for INCLUDE statements
    if ( text.trimmed().startsWith( "INCLUDE" ) )
    {
        // Find the file path after INCLUDE keyword
        QRegularExpression includeRegex( "^\\s*INCLUDE\\s+(['\"]?)([^'\"\\s]+)\\1" );
        QRegularExpressionMatch match = includeRegex.match( text );
        if ( match.hasMatch() )
        {
            int pathStart = match.capturedStart( 2 );
            int pathLength = match.capturedLength( 2 );
            setFormat( pathStart, pathLength, m_includePathFormat );
        }
    }

    // Apply keyword highlighting with context awareness
    highlightKeywords( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataFileSyntaxHighlighter::initializeKeywordSets()
{
    if ( m_keywordDatabase )
    {
        // Get section keywords
        QStringList sections = m_keywordDatabase->getAllSections();
        for ( const QString& section : sections )
        {
            m_sectionKeywords.insert( section );
        }

        // Get all valid keywords
        QStringList keywords = m_keywordDatabase->getAllKeywords();
        for ( const QString& keyword : keywords )
        {
            m_validKeywords.insert( keyword );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataFileSyntaxHighlighter::highlightKeywords( const QString& text )
{
    // Find keywords at line start (after optional whitespace)
    static QRegularExpression keywordPattern( "^\\s*([A-Z][_A-Z0-9]*)\\b" );
    QRegularExpressionMatch match = keywordPattern.match( text );

    if ( match.hasMatch() )
    {
        QString keyword = match.captured( 1 );
        int start = match.capturedStart( 1 );
        int length = match.capturedLength( 1 );

        QTextCharFormat format;

        if ( m_sectionKeywords.contains( keyword ) )
        {
            // Section keyword
            format = m_sectionKeywordFormat;
        }
        else if ( m_validKeywords.contains( keyword ) )
        {
            // Check if keyword is valid in current context
            QString currentSection = getCurrentSection( currentBlock().blockNumber() );
            if ( !currentSection.isEmpty() )
            {
                KeywordInfo info = m_keywordDatabase->getKeywordInfo( keyword );
                if ( info.isValidInSection( currentSection ) )
                {
                    format = m_keywordFormat; // Valid in context
                }
                else
                {
                    format = m_contextInvalidFormat; // Valid keyword, wrong section
                }
            }
            else
            {
                format = m_keywordFormat; // No section context, assume valid
            }
        }
        else
        {
            // Unknown keyword
            format = m_invalidKeywordFormat;
        }

        setFormat( start, length, format );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString DataFileSyntaxHighlighter::getCurrentSection( int blockNumber ) const
{
    // Search backwards from current block to find the last section keyword
    QTextDocument* doc = document();
    if ( !doc ) return QString();

    for ( int i = blockNumber; i >= 0; --i )
    {
        QTextBlock block = doc->findBlockByNumber( i );
        if ( block.isValid() )
        {
            QString blockText = block.text().trimmed();
            for ( const QString& section : m_sectionKeywords )
            {
                if ( blockText.startsWith( section, Qt::CaseInsensitive ) )
                {
                    return section;
                }
            }
        }
    }

    return QString();
}
