#include "DataFileCompleter.h"
#include "KeywordDatabase.h"

#include <QTextCursor>
#include <QTextBlock>
#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
DataFileCompleter::DataFileCompleter(QObject* parent)
    : QCompleter(parent)
    , m_keywordDatabase(KeywordDatabase::instance())
    , m_model(new QStringListModel(this))
{
    setModel(m_model);
    setCaseSensitivity(Qt::CaseInsensitive);
    setWrapAround(false);
    setCompletionMode(QCompleter::PopupCompletion);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataFileCompleter::updateCompletions(const QTextCursor& cursor)
{
    // Get current section context
    m_currentSection = getCurrentSection(cursor);
    
    // Get word under cursor for prefix
    QTextCursor wordCursor = cursor;
    wordCursor.select(QTextCursor::WordUnderCursor);
    QString currentWord = wordCursor.selectedText();
    
    // If we're at the beginning of a line or after whitespace, suggest keywords
    QTextCursor lineCursor = cursor;
    lineCursor.movePosition(QTextCursor::StartOfLine);
    QString lineText = lineCursor.block().text().left(cursor.positionInBlock());
    
    // Check if we're at a position where keywords are expected
    static QRegularExpression keywordPosition("^\\s*[A-Z]*$");
    if (keywordPosition.match(lineText).hasMatch())
    {
        // Get completions from keyword database
        QStringList completions = m_keywordDatabase->getCompletions(currentWord, m_currentSection);
        updateModel(completions);
    }
    else
    {
        // Clear completions if not in a keyword position
        updateModel(QStringList());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString DataFileCompleter::getCurrentSection(const QTextCursor& cursor) const
{
    // Search backwards from current position to find the last section keyword
    QTextCursor searchCursor = cursor;
    searchCursor.movePosition(QTextCursor::Start);
    
    QString currentSection;
    QStringList sections = m_keywordDatabase->getAllSections();
    
    // Iterate through all blocks from start to current position
    QTextBlock block = searchCursor.block();
    QTextBlock currentBlock = cursor.block();
    
    while (block.isValid() && block.blockNumber() <= currentBlock.blockNumber())
    {
        QString blockText = block.text().trimmed();
        
        // Check if this line contains a section keyword
        for (const QString& section : sections)
        {
            if (blockText.startsWith(section, Qt::CaseInsensitive))
            {
                currentSection = section;
                break;
            }
        }
        
        block = block.next();
    }
    
    return currentSection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataFileCompleter::updateModel(const QStringList& completions)
{
    m_model->setStringList(completions);
}