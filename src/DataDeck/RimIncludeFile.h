#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildField.h"

#include <QFileInfo>

class RimDataDeck;

//==================================================================================================
/// Represents an included file referenced by an INCLUDE keyword in Eclipse DATA files
//==================================================================================================
class RimIncludeFile : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIncludeFile();
    ~RimIncludeFile() override;

    void setIncludePath(const QString& path, const QString& basePath = QString());
    QString includePath() const;
    QString resolvedPath() const;
    bool fileExists() const;
    QString fileName() const;
    
    bool loadContent();
    RimDataDeck* content() const;
    
    void updateFileStatus();

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    QString resolveAbsolutePath(const QString& includePath, const QString& basePath) const;

private:
    caf::PdmField<QString>                  m_includePath;      // Original path from INCLUDE statement
    caf::PdmField<QString>                  m_resolvedPath;     // Computed absolute path
    caf::PdmField<bool>                     m_fileExists;       // File existence status
    caf::PdmField<QString>                  m_fileName;         // Display name
    caf::PdmField<QString>                  m_basePath;         // Base directory for relative paths
    
    caf::PdmChildField<RimDataDeck*>        m_content;          // Parsed content of included file
};