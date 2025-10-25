#include "RimDataKeyword.h"
#include "RimDataItem.h"

#include "cafPdmUiOrdering.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

CAF_PDM_SOURCE_INIT( RimDataKeyword, "DataKeyword" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataKeyword::RimDataKeyword()
    : m_deckKeyword( nullptr )
{
    CAF_PDM_InitObject( "Keyword", "", "", "" );

    CAF_PDM_InitField( &m_keywordName, "KeywordName", QString( "" ), "Keyword", "", "", "" );
    m_keywordName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_recordCount, "RecordCount", 0, "Records", "", "", "" );
    m_recordCount.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_isLargeArray, "IsLargeArray", false, "Is Large Array", "", "", "" );
    m_isLargeArray.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_summary, "Summary", QString( "" ), "Summary", "", "", "" );
    m_summary.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_items, "Items", "Items", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataKeyword::~RimDataKeyword()
{
    m_items.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataKeyword::setDeckKeyword( const Opm::DeckKeyword* deckKeyword )
{
    m_deckKeyword = deckKeyword;

    if ( m_deckKeyword )
    {
        m_keywordName = QString::fromStdString( m_deckKeyword->name() );
        m_recordCount = static_cast<int>( m_deckKeyword->size() );

        // Calculate total items across all records
        size_t totalItems = 0;
        for ( size_t i = 0; i < m_deckKeyword->size(); ++i )
        {
            const auto& record = m_deckKeyword->getRecord( i );
            totalItems += record.size();
        }

        // Determine if this is a large array
        m_isLargeArray = ( totalItems > LARGE_ARRAY_THRESHOLD );

        if ( m_isLargeArray )
        {
            // For large arrays, just show summary
            m_summary = generateSummary();
        }
        else
        {
            // For normal keywords, build item tree
            buildItemsFromKeyword();
            m_summary = "";
        }

        // Update UI name to show keyword name
        setUiName( m_keywordName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataKeyword::keywordName() const
{
    return m_keywordName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDataKeyword::recordCount() const
{
    return m_recordCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataKeyword::isLargeArray() const
{
    return m_isLargeArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataKeyword::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_keywordName );
    uiOrdering.add( &m_recordCount );

    if ( m_isLargeArray )
    {
        uiOrdering.add( &m_summary );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataKeyword::buildItemsFromKeyword()
{
    m_items.deleteChildren();

    if ( !m_deckKeyword )
    {
        return;
    }

    // Limit display to first few records if there are many
    const size_t maxRecordsToShow = 20;
    size_t recordsToShow = std::min( m_deckKeyword->size(), maxRecordsToShow );

    for ( size_t recIdx = 0; recIdx < recordsToShow; ++recIdx )
    {
        const auto& record = m_deckKeyword->getRecord( recIdx );

        for ( size_t itemIdx = 0; itemIdx < record.size(); ++itemIdx )
        {
            const auto& deckItem = record.getItem( itemIdx );

            RimDataItem* item = new RimDataItem();
            item->setDeckItem( &deckItem );

            // Set UI name to show record index if multiple records
            if ( m_deckKeyword->size() > 1 )
            {
                item->setUiName( QString( "Record %1: %2" ).arg( recIdx + 1 ).arg( item->itemName() ) );
            }
            else
            {
                item->setUiName( item->itemName() );
            }

            m_items.push_back( item );
        }
    }

    if ( m_deckKeyword->size() > maxRecordsToShow )
    {
        m_summary = QString( "Showing first %1 of %2 records" ).arg( maxRecordsToShow ).arg( m_deckKeyword->size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataKeyword::generateSummary() const
{
    if ( !m_deckKeyword )
    {
        return QString( "" );
    }

    // Count total data items
    size_t totalItems = 0;
    for ( size_t i = 0; i < m_deckKeyword->size(); ++i )
    {
        const auto& record = m_deckKeyword->getRecord( i );
        totalItems += record.size();
    }

    // Try to determine data type from first item
    QString dataType = "MIXED";
    if ( m_deckKeyword->size() > 0 )
    {
        const auto& record = m_deckKeyword->getRecord( 0 );
        if ( record.size() > 0 )
        {
            const auto& item = record.getItem( 0 );
            if ( item.getType() == Opm::type_tag::integer )
            {
                dataType = "INTEGER";
            }
            else if ( item.getType() == Opm::type_tag::fdouble )
            {
                dataType = "DOUBLE";
            }
            else if ( item.getType() == Opm::type_tag::string )
            {
                dataType = "STRING";
            }
        }
    }

    return QString( "Large array: %1 items, %2 records, Type: %3" )
        .arg( totalItems )
        .arg( m_recordCount() )
        .arg( dataType );
}
