#include "DataFileSyntaxHighlighter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
DataFileSyntaxHighlighter::DataFileSyntaxHighlighter( QTextDocument* parent )
    : QSyntaxHighlighter( parent )
{
    HighlightingRule rule;

    // Comments - must be first to take precedence
    m_commentFormat.setForeground( QColor( 106, 153, 85 ) ); // Green like in example
    m_commentFormat.setFontItalic( true );

    // Section keywords (RUNSPEC, GRID, PROPS, etc.)
    m_sectionKeywordFormat.setForeground( QColor( 86, 156, 214 ) ); // Blue like in example
    m_sectionKeywordFormat.setFontWeight( QFont::Bold );
    QStringList sectionKeywords;
    sectionKeywords << "^RUNSPEC\\b"
                    << "^GRID\\b"
                    << "^EDIT\\b"
                    << "^PROPS\\b"
                    << "^REGIONS\\b"
                    << "^SOLUTION\\b"
                    << "^SUMMARY\\b"
                    << "^SCHEDULE\\b";
    for ( const QString& pattern : sectionKeywords )
    {
        rule.pattern = QRegularExpression( pattern );
        rule.format  = m_sectionKeywordFormat;
        m_rules.append( rule );
    }

    // INCLUDE keyword (important)
    QTextCharFormat includeFormat;
    includeFormat.setForeground( QColor( 197, 134, 192 ) ); // Purple/magenta like in example
    includeFormat.setFontWeight( QFont::Bold );
    rule.pattern = QRegularExpression( "^INCLUDE\\b" );
    rule.format  = includeFormat;
    m_rules.append( rule );

    // Regular keywords (must start at line beginning or after whitespace)
    m_keywordFormat.setForeground( QColor( 215, 186, 125 ) ); // Orange/yellow like in example
    m_keywordFormat.setFontWeight( QFont::Bold );
    rule.pattern = QRegularExpression( "^[A-Z][_A-Z0-9]*\\b" );
    rule.format  = m_keywordFormat;
    m_rules.append( rule );

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

    // Apply other rules
    for ( const HighlightingRule& rule : m_rules )
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch( text );
        while ( matchIterator.hasNext() )
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat( match.capturedStart(), match.capturedLength(), rule.format );
        }
    }
}
