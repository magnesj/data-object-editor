#include "RimIncludeFile.h"

#include "RimDataDeck.h"

#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RimIncludeFile, "IncludeFile");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIncludeFile::RimIncludeFile()
{
    CAF_PDM_InitObject("Include File", ":/File16x16.png");

    CAF_PDM_InitField(&m_includePath, "IncludePath", QString(), "Include Path");
    m_includePath.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_resolvedPath, "ResolvedPath", QString(), "Resolved Path");
    m_resolvedPath.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_fileExists, "FileExists", false, "File Exists");
    m_fileExists.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_fileName, "FileName", QString(), "File Name");
    m_fileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_basePath, "BasePath", QString(), "Base Path");
    m_basePath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_content, "Content", "Content");
    m_content.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIncludeFile::~RimIncludeFile()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeFile::setIncludePath(const QString& path, const QString& basePath)
{
    m_includePath = path;
    m_basePath = basePath;
    
    QString resolved = resolveAbsolutePath(path, basePath);
    m_resolvedPath = resolved;
    
    QFileInfo info(resolved);
    m_fileName = info.fileName();
    
    updateFileStatus();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimIncludeFile::includePath() const
{
    return m_includePath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimIncludeFile::resolvedPath() const
{
    return m_resolvedPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIncludeFile::fileExists() const
{
    return m_fileExists;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimIncludeFile::fileName() const
{
    return m_fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIncludeFile::loadContent()
{
    if (!m_fileExists)
    {
        return false;
    }

    if (m_content == nullptr)
    {
        m_content = new RimDataDeck();
    }

    return m_content->loadFromFile(m_resolvedPath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDataDeck* RimIncludeFile::content() const
{
    return m_content;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeFile::updateFileStatus()
{
    QFileInfo info(m_resolvedPath);
    m_fileExists = info.exists() && info.isFile();
    
    // Update UI name to show status
    QString uiName = m_fileName;
    if (!m_fileExists)
    {
        uiName += " (Missing)";
        setUiIconFromResourceString(":/Warning16x16.png");
    }
    else
    {
        uiName += " (Available)";
        setUiIconFromResourceString(":/File16x16.png");
    }
    setUiName(uiName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeFile::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_fileName);
    uiOrdering.add(&m_includePath);
    uiOrdering.add(&m_resolvedPath);
    uiOrdering.add(&m_fileExists);
    
    // Show content if file exists and is loaded
    if (m_fileExists && m_content)
    {
        caf::PdmUiGroup* contentGroup = uiOrdering.addNewGroup("File Content");
        contentGroup->add(&m_content);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeFile::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    // Show the content as a child node if file exists and is loaded
    if (m_fileExists && m_content)
    {
        uiTreeOrdering.add(&m_content);
    }
    
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimIncludeFile::resolveAbsolutePath(const QString& includePath, const QString& basePath) const
{
    QFileInfo info(includePath);
    if (info.isAbsolute())
    {
        return includePath;
    }

    if (basePath.isEmpty())
    {
        return includePath;
    }

    return QDir(basePath).absoluteFilePath(includePath);
}