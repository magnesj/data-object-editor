#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmProxyValueField.h"

#include <memory>

namespace Opm
{
class DeckKeyword;
}

class RimDataItem;

//==================================================================================================
/// Represents a keyword from an Eclipse DATA file
//==================================================================================================
class RimDataKeyword : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataKeyword();
    ~RimDataKeyword() override;

    virtual void setDeckKeyword( const Opm::DeckKeyword* deckKeyword );

    QString keywordName() const;
    int     recordCount() const;
    bool    isLargeArray() const;

    // Text position tracking
    void    setTextPosition( int startLine, int endLine );
    int     startLine() const;
    int     endLine() const;

    static constexpr size_t LARGE_ARRAY_THRESHOLD = 100;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    
    const Opm::DeckKeyword* deckKeyword() const { return m_deckKeyword; }

private:
    void buildItemsFromKeyword();
    QString generateSummary() const;
    QString formatKeywordContent() const;

private:
    caf::PdmField<QString>                      m_keywordName;
    caf::PdmField<int>                          m_recordCount;
    caf::PdmField<bool>                         m_isLargeArray;
    caf::PdmField<QString>                      m_summary;
    caf::PdmProxyValueField<QString>            m_content;

    // Text position tracking
    caf::PdmField<int>                          m_startLine;
    caf::PdmField<int>                          m_endLine;

    caf::PdmChildArrayField<RimDataItem*>       m_items;

    const Opm::DeckKeyword*                     m_deckKeyword;
};
