#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"

#include <memory>
#include <QMap>
#include <QPair>
#include <QSet>

namespace Opm
{
class Deck;
}

class RimDataSection;
class RimDataKeyword;
class RimIncludeFile;

//==================================================================================================
/// Represents an Eclipse DATA file parsed using opm-common
//==================================================================================================
class RimDataDeck : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataDeck();
    ~RimDataDeck() override;

    bool loadFromFile( const QString& filePath );
    void setDeck( std::shared_ptr<Opm::Deck> deck, const QString& filePath );
    bool updateFromDeck( std::shared_ptr<Opm::Deck> deck );

    QString             filePath() const;
    int                 keywordCount() const;
    std::shared_ptr<Opm::Deck> deck() const;

    QString serializeToText() const;
    
    // Position tracking
    RimDataKeyword* findKeywordAtLine( int lineNumber );
    
    // Include file management
    void addIncludeFile( RimIncludeFile* includeFile );
    QList<RimIncludeFile*> includeFiles() const;
    void resolveIncludes();
    QString basePath() const;
    QStringList findIncludeReferences() const;
    bool validateIncludePaths() const;
    void resolveIncludesFromRawFile();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    void buildSectionsFromDeck();
    void calculateTextPositions();

private:
    caf::PdmField<QString>                      m_filePath;
    caf::PdmField<int>                          m_keywordCount;
    caf::PdmField<QString>                      m_fileName;
    caf::PdmField<QString>                      m_basePath;        // Base directory for resolving relative includes

    caf::PdmChildArrayField<RimDataSection*>    m_sections;
    caf::PdmChildArrayField<RimIncludeFile*>    m_includeFiles;   // Managed include files

    std::shared_ptr<Opm::Deck>                  m_deck;
    
    // Position tracking: maps keyword index to (startLine, endLine)
    QMap<size_t, QPair<int, int>>               m_keywordPositions;
};
