#pragma once

#include <QCompleter>
#include <QStringListModel>
#include <QTextCursor>

class KeywordDatabase;

//==================================================================================================
/// Custom completer for Eclipse DATA files with context-aware keyword completion
//==================================================================================================
class DataFileCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit DataFileCompleter(QObject* parent = nullptr);
    
    void updateCompletions(const QTextCursor& cursor);
    QString getCurrentSection(const QTextCursor& cursor) const;
    
private:
    void updateModel(const QStringList& completions);
    
    KeywordDatabase* m_keywordDatabase;
    QStringListModel* m_model;
    QString m_currentSection;
};