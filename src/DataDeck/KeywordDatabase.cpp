#include "KeywordDatabase.h"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

KeywordDatabase* KeywordDatabase::s_instance = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
KeywordDatabase::KeywordDatabase(QObject* parent)
    : QObject(parent)
{
    // Initialize standard section keywords
    m_sectionKeywords << "RUNSPEC" << "GRID" << "EDIT" << "PROPS" 
                     << "REGIONS" << "SOLUTION" << "SUMMARY" << "SCHEDULE";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
KeywordDatabase* KeywordDatabase::instance()
{
    if (!s_instance)
    {
        s_instance = new KeywordDatabase();
        s_instance->loadKeywords();
    }
    return s_instance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordDatabase::loadKeywords()
{
    QString keywordsDir = findKeywordsDirectory();
    if (keywordsDir.isEmpty())
    {
        qWarning() << "Could not find keywords directory, using fallback keywords";
        loadFallbackKeywords();
        return;
    }
    
    qDebug() << "Loading keywords from:" << keywordsDir;
    loadKeywordsFromDirectory(keywordsDir);
    qDebug() << "Loaded" << m_keywords.size() << "keywords";
    
    // If no keywords loaded, use fallback
    if (m_keywords.isEmpty())
    {
        qWarning() << "No keywords loaded from JSON files, using fallback keywords";
        loadFallbackKeywords();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString KeywordDatabase::findKeywordsDirectory() const
{
    // Try to find the keywords directory relative to application
    QStringList searchPaths;
    
    QString appDir = QApplication::applicationDirPath();
    
    // Common relative paths from executable location
    searchPaths << appDir + "/../external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    searchPaths << appDir + "/../../external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    searchPaths << appDir + "/../../../external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    
    // From current working directory (useful during development)
    searchPaths << "external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    searchPaths << "../external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    searchPaths << "../../external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    
    // Try absolute path from known working directory structure
    QDir workingDir = QDir::current();
    workingDir.cdUp(); // Go up from build directory
    QString rootPath = workingDir.absolutePath() + "/external/ResInsight/ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/share/keywords";
    searchPaths << rootPath;
    
    qDebug() << "Searching for keywords directory...";
    qDebug() << "Application directory:" << appDir;
    qDebug() << "Current working directory:" << QDir::current().absolutePath();
    
    for (const QString& path : searchPaths)
    {
        QDir dir(path);
        qDebug() << "Checking path:" << dir.absolutePath();
        if (dir.exists() && dir.exists("000_Eclipse100"))
        {
            qDebug() << "Found keywords directory at:" << dir.absolutePath();
            return dir.absolutePath();
        }
    }
    
    qWarning() << "Keywords directory not found in any of the search paths";
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordDatabase::loadFallbackKeywords()
{
    // Add some common Eclipse keywords as fallback
    struct FallbackKeyword {
        QString name;
        QStringList sections;
        QString type;
    };
    
    QList<FallbackKeyword> fallbackKeywords = {
        // RUNSPEC section
        {"DIMENS", {"RUNSPEC"}, "INT"},
        {"TABDIMS", {"RUNSPEC"}, "INT"},
        {"EQLDIMS", {"RUNSPEC"}, "INT"},
        {"WELLDIMS", {"RUNSPEC"}, "INT"},
        {"START", {"RUNSPEC"}, "DATE"},
        {"OIL", {"RUNSPEC"}, "NONE"},
        {"WATER", {"RUNSPEC"}, "NONE"},
        {"GAS", {"RUNSPEC"}, "NONE"},
        {"METRIC", {"RUNSPEC"}, "NONE"},
        {"FIELD", {"RUNSPEC"}, "NONE"},
        
        // GRID section
        {"DX", {"GRID"}, "DOUBLE"},
        {"DY", {"GRID"}, "DOUBLE"},
        {"DZ", {"GRID"}, "DOUBLE"},
        {"TOPS", {"GRID"}, "DOUBLE"},
        {"PORO", {"GRID"}, "DOUBLE"},
        {"PERMX", {"GRID"}, "DOUBLE"},
        {"PERMY", {"GRID"}, "DOUBLE"},
        {"PERMZ", {"GRID"}, "DOUBLE"},
        {"COORD", {"GRID"}, "DOUBLE"},
        {"ZCORN", {"GRID"}, "DOUBLE"},
        {"ACTNUM", {"GRID"}, "INT"},
        
        // PROPS section
        {"SWOF", {"PROPS"}, "DOUBLE"},
        {"SGOF", {"PROPS"}, "DOUBLE"},
        {"PVTO", {"PROPS"}, "DOUBLE"},
        {"PVTW", {"PROPS"}, "DOUBLE"},
        {"PVDG", {"PROPS"}, "DOUBLE"},
        {"ROCK", {"PROPS"}, "DOUBLE"},
        {"DENSITY", {"PROPS"}, "DOUBLE"},
        
        // SOLUTION section
        {"EQUIL", {"SOLUTION"}, "DOUBLE"},
        {"PRESSURE", {"SOLUTION"}, "DOUBLE"},
        {"SWAT", {"SOLUTION"}, "DOUBLE"},
        {"SGAS", {"SOLUTION"}, "DOUBLE"},
        
        // SCHEDULE section
        {"DATES", {"SCHEDULE"}, "DATE"},
        {"WELSPECS", {"SCHEDULE"}, "STRING"},
        {"COMPDAT", {"SCHEDULE"}, "MIXED"},
        {"WCONPROD", {"SCHEDULE"}, "MIXED"},
        {"WCONINJE", {"SCHEDULE"}, "MIXED"},
        {"TSTEP", {"SCHEDULE"}, "DOUBLE"},
        
        // Common to multiple sections
        {"INCLUDE", {"RUNSPEC", "GRID", "EDIT", "PROPS", "REGIONS", "SOLUTION", "SUMMARY", "SCHEDULE"}, "STRING"}
    };
    
    for (const FallbackKeyword& kw : fallbackKeywords)
    {
        KeywordInfo info;
        info.name = kw.name;
        info.validSections = kw.sections;
        info.valueType = kw.type;
        m_keywords[kw.name] = info;
    }
    
    qDebug() << "Loaded" << m_keywords.size() << "fallback keywords";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordDatabase::loadKeywordsFromDirectory(const QString& directory)
{
    // Load from Eclipse100 directory (main keywords)
    QString eclipse100Dir = directory + "/000_Eclipse100";
    QDir dir(eclipse100Dir);
    
    if (!dir.exists())
    {
        qWarning() << "Eclipse100 directory not found:" << eclipse100Dir;
        return;
    }
    
    // Iterate through A-Z subdirectories
    QDirIterator dirIt(eclipse100Dir, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext())
    {
        QString subdir = dirIt.next();
        QDirIterator fileIt(subdir, QDir::Files);
        
        while (fileIt.hasNext())
        {
            QString filePath = fileIt.next();
            QFileInfo fileInfo(filePath);
            QString keywordName = fileInfo.baseName();
            
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly))
            {
                QByteArray data = file.readAll();
                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson(data, &error);
                
                if (error.error == QJsonParseError::NoError && doc.isObject())
                {
                    KeywordInfo info = parseKeywordJson(doc.object(), keywordName);
                    if (!info.name.isEmpty())
                    {
                        m_keywords[info.name] = info;
                    }
                }
                else
                {
                    qDebug() << "Failed to parse JSON for keyword" << keywordName << ":" << error.errorString();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
KeywordInfo KeywordDatabase::parseKeywordJson(const QJsonObject& json, const QString& keywordName)
{
    KeywordInfo info;
    info.name = keywordName;
    
    // Parse sections
    if (json.contains("sections") && json["sections"].isArray())
    {
        QJsonArray sections = json["sections"].toArray();
        for (const QJsonValue& section : sections)
        {
            info.validSections << section.toString();
        }
    }
    
    // Parse data type information
    if (json.contains("data") && json["data"].isObject())
    {
        QJsonObject data = json["data"].toObject();
        if (data.contains("value_type"))
        {
            info.valueType = data["value_type"].toString();
        }
        
        // Parse items/parameters if present
        if (data.contains("items") && data["items"].isArray())
        {
            QJsonArray items = data["items"].toArray();
            for (const QJsonValue& item : items)
            {
                if (item.isObject())
                {
                    QJsonObject itemObj = item.toObject();
                    if (itemObj.contains("name"))
                    {
                        info.parameterNames << itemObj["name"].toString();
                    }
                    if (itemObj.contains("value_type"))
                    {
                        info.parameterTypes << itemObj["value_type"].toString();
                    }
                }
            }
        }
    }
    
    // Parse size information
    if (json.contains("size"))
    {
        info.hasSize = true;
        if (json["size"].isObject())
        {
            QJsonObject sizeObj = json["size"].toObject();
            if (sizeObj.contains("keyword"))
            {
                info.sizeKeyword = sizeObj["keyword"].toString();
            }
        }
    }
    
    return info;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool KeywordDatabase::hasKeyword(const QString& keyword) const
{
    return m_keywords.contains(keyword.toUpper());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
KeywordInfo KeywordDatabase::getKeywordInfo(const QString& keyword) const
{
    return m_keywords.value(keyword.toUpper());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList KeywordDatabase::getAllKeywords() const
{
    return m_keywords.keys();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList KeywordDatabase::getKeywordsForSection(const QString& section) const
{
    QStringList result;
    
    for (auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
    {
        const KeywordInfo& info = it.value();
        if (info.isValidInSection(section))
        {
            result << info.name;
        }
    }
    
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList KeywordDatabase::getCompletions(const QString& prefix, const QString& currentSection) const
{
    QStringList result;
    QString upperPrefix = prefix.toUpper();
    
    // Add section keywords first if no section specified
    if (currentSection.isEmpty())
    {
        for (const QString& section : m_sectionKeywords)
        {
            if (section.startsWith(upperPrefix))
            {
                result << section;
            }
        }
    }
    
    // Add keywords valid for current section
    for (auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
    {
        const KeywordInfo& info = it.value();
        if (info.name.startsWith(upperPrefix))
        {
            if (currentSection.isEmpty() || info.isValidInSection(currentSection))
            {
                result << info.name;
            }
        }
    }
    
    result.sort();
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList KeywordDatabase::getAllSections() const
{
    return m_sectionKeywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool KeywordDatabase::isSection(const QString& keyword) const
{
    return m_sectionKeywords.contains(keyword.toUpper());
}