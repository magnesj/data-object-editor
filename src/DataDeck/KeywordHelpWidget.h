#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>

struct KeywordInfo;
class KeywordDatabase;

//==================================================================================================
/// Widget for displaying help information about Eclipse DATA file keywords
//==================================================================================================
class KeywordHelpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeywordHelpWidget(QWidget* parent = nullptr);
    
public slots:
    void showKeywordHelp(const QString& keyword, const QString& currentSection = QString());
    void clearHelp();
    
private:
    void setupUI();
    void formatKeywordInfo(const KeywordInfo& info, const QString& currentSection);
    QString getSectionDescription(const QString& section) const;
    
    KeywordDatabase* m_keywordDatabase;
    QLabel* m_titleLabel;
    QTextEdit* m_helpText;
    QVBoxLayout* m_layout;
};