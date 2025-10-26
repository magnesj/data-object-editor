#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QSet>

class KeywordDatabase;

//==================================================================================================
/// Syntax highlighter for Eclipse DATA files with dynamic keyword support
//==================================================================================================
class DataFileSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit DataFileSyntaxHighlighter( QTextDocument* parent = nullptr );

protected:
    void highlightBlock( const QString& text ) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    void initializeKeywordSets();
    void highlightKeywords( const QString& text );
    QString getCurrentSection( int blockNumber ) const;

    QVector<HighlightingRule> m_rules;
    KeywordDatabase* m_keywordDatabase;
    QSet<QString> m_sectionKeywords;
    QSet<QString> m_validKeywords;

    QTextCharFormat m_sectionKeywordFormat;
    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_invalidKeywordFormat;
    QTextCharFormat m_contextInvalidFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_delimiterFormat;
    QTextCharFormat m_includeFormat;
    QTextCharFormat m_includePathFormat;
};
