#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>

//==================================================================================================
/// Information about a single Eclipse DATA keyword
//==================================================================================================
struct KeywordInfo
{
    QString name;
    QStringList validSections;
    QString valueType;
    QString description;
    QStringList parameterNames;
    QStringList parameterTypes;
    QStringList parameterDescriptions;
    bool hasSize = false;
    QString sizeKeyword;
    
    bool isValidInSection(const QString& section) const {
        return validSections.isEmpty() || validSections.contains(section, Qt::CaseInsensitive);
    }
};

//==================================================================================================
/// Database of Eclipse DATA file keywords loaded from opm-common JSON definitions
//==================================================================================================
class KeywordDatabase : public QObject
{
    Q_OBJECT

public:
    static KeywordDatabase* instance();
    
    void loadKeywords();
    
    // Keyword lookup
    bool hasKeyword(const QString& keyword) const;
    KeywordInfo getKeywordInfo(const QString& keyword) const;
    QStringList getAllKeywords() const;
    QStringList getKeywordsForSection(const QString& section) const;
    QStringList getCompletions(const QString& prefix, const QString& currentSection = QString()) const;
    
    // Section detection
    QStringList getAllSections() const;
    bool isSection(const QString& keyword) const;
    
private:
    explicit KeywordDatabase(QObject* parent = nullptr);
    
    void loadKeywordsFromDirectory(const QString& directory);
    void loadFallbackKeywords();
    KeywordInfo parseKeywordJson(const QJsonObject& json, const QString& keywordName);
    QString findKeywordsDirectory() const;
    
    static KeywordDatabase* s_instance;
    QMap<QString, KeywordInfo> m_keywords;
    QStringList m_sectionKeywords;
};