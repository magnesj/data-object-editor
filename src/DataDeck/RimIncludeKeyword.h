#pragma once

#include "RimDataKeyword.h"
#include "cafPdmChildField.h"

class RimIncludeFile;

//==================================================================================================
/// Represents an INCLUDE keyword from an Eclipse DATA file
//==================================================================================================
class RimIncludeKeyword : public RimDataKeyword
{
    CAF_PDM_HEADER_INIT;

public:
    RimIncludeKeyword();
    ~RimIncludeKeyword() override;

    void setIncludeFile(RimIncludeFile* includeFile);
    RimIncludeFile* includeFile() const;
    
    QString includePath() const;
    bool isIncludeResolved() const;
    
    // Override to handle INCLUDE-specific logic
    void setDeckKeyword(const Opm::DeckKeyword* deckKeyword) override;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    void extractIncludePathFromKeyword();

private:
    caf::PdmField<QString>                  m_includePath;      // Path from INCLUDE statement
    caf::PdmField<bool>                     m_isResolved;       // Whether include file was found and loaded
    caf::PdmChildField<RimIncludeFile*>     m_includeFile;      // Associated include file
};