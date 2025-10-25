#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"

#include <memory>

namespace Opm
{
class DeckItem;
}

//==================================================================================================
/// Represents a single data item from an Eclipse DATA file keyword
//==================================================================================================
class RimDataItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataItem();
    ~RimDataItem() override;

    void setDeckItem( const Opm::DeckItem* deckItem );

    QString itemName() const;
    QString dataType() const;
    QString valueAsString() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    QString formatValue() const;

private:
    caf::PdmField<QString>               m_itemName;
    caf::PdmField<QString>               m_dataType;
    caf::PdmProxyValueField<QString>     m_value;

    const Opm::DeckItem*                 m_deckItem;
};
