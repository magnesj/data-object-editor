#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

//==================================================================================================
/// Syntax highlighter for Eclipse DATA files
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

    QVector<HighlightingRule> m_rules;

    QTextCharFormat m_sectionKeywordFormat;
    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_delimiterFormat;
};
