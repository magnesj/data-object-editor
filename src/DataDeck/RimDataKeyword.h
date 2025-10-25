#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"

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

    void setDeckKeyword( const Opm::DeckKeyword* deckKeyword );

    QString keywordName() const;
    int     recordCount() const;
    bool    isLargeArray() const;

    static constexpr size_t LARGE_ARRAY_THRESHOLD = 100;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void buildItemsFromKeyword();
    QString generateSummary() const;

private:
    caf::PdmField<QString>                      m_keywordName;
    caf::PdmField<int>                          m_recordCount;
    caf::PdmField<bool>                         m_isLargeArray;
    caf::PdmField<QString>                      m_summary;

    caf::PdmChildArrayField<RimDataItem*>       m_items;

    const Opm::DeckKeyword*                     m_deckKeyword;
};
