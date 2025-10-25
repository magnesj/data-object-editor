#include "RimDataKeyword.h"
#include "RimDataItem.h"

#include "cafPdmUiOrdering.h"
#include "cafPdmUiTextEditor.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

#include <QFont>

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

    CAF_PDM_InitFieldNoDefault( &m_content, "Content", "Keyword Content", "", "", "" );
    m_content.registerGetMethod( this, &RimDataKeyword::formatKeywordContent );
    m_content.uiCapability()->setUiReadOnly( true );
    m_content.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    // Text position tracking
    CAF_PDM_InitField( &m_startLine, "StartLine", -1, "Start Line", "", "", "" );
    m_startLine.uiCapability()->setUiHidden( true );
    
    CAF_PDM_InitField( &m_endLine, "EndLine", -1, "End Line", "", "", "" );
    m_endLine.uiCapability()->setUiHidden( true );

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
void RimDataKeyword::setTextPosition( int startLine, int endLine )
{
    m_startLine = startLine;
    m_endLine = endLine;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDataKeyword::startLine() const
{
    return m_startLine;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDataKeyword::endLine() const
{
    return m_endLine;
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
    else
    {
        // For non-large arrays, show the formatted content
        uiOrdering.add( &m_content );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataKeyword::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_content )
    {
        auto* textEditAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( textEditAttr )
        {
            textEditAttr->textMode = caf::PdmUiTextEditorAttribute::PLAIN;
            textEditAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            textEditAttr->heightHint = 300; // Height in pixels

            // Use monospace font for better alignment
            QFont font( "Courier" );
            font.setStyleHint( QFont::Monospace );
            font.setPointSize( 9 );
            textEditAttr->font = font;
        }
    }
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataKeyword::formatKeywordContent() const
{
    if ( !m_deckKeyword )
    {
        return QString( "" );
    }

    QStringList lines;

    // Add keyword header
    lines.append( QString( "%1" ).arg( m_keywordName() ) );

    // Limit number of records to display
    const size_t maxRecordsToShow = 50;
    size_t recordsToShow = std::min( m_deckKeyword->size(), maxRecordsToShow );

    for ( size_t recIdx = 0; recIdx < recordsToShow; ++recIdx )
    {
        const auto& record = m_deckKeyword->getRecord( recIdx );

        QStringList itemValues;

        for ( size_t itemIdx = 0; itemIdx < record.size(); ++itemIdx )
        {
            const auto& item = record.getItem( itemIdx );

            if ( !item.hasValue( 0 ) )
            {
                itemValues.append( "*" ); // Defaulted
                continue;
            }

            try
            {
                if ( item.getType() == Opm::type_tag::integer )
                {
                    itemValues.append( QString::number( item.get<int>( 0 ) ) );
                }
                else if ( item.getType() == Opm::type_tag::fdouble )
                {
                    itemValues.append( QString::number( item.get<double>( 0 ), 'g', 10 ) );
                }
                else if ( item.getType() == Opm::type_tag::string )
                {
                    QString strValue = QString::fromStdString( item.get<std::string>( 0 ) );
                    // Quote strings if they contain spaces or are keywords
                    if ( strValue.contains( ' ' ) || strValue.isEmpty() )
                    {
                        itemValues.append( QString( "'%1'" ).arg( strValue ) );
                    }
                    else
                    {
                        itemValues.append( strValue );
                    }
                }
            }
            catch ( ... )
            {
                itemValues.append( "<error>" );
            }
        }

        // Format record line
        QString recordLine = "  " + itemValues.join( "  " );

        // Add trailing / for Eclipse format
        if ( !recordLine.trimmed().isEmpty() )
        {
            recordLine += "  /";
        }

        lines.append( recordLine );
    }

    if ( m_deckKeyword->size() > maxRecordsToShow )
    {
        lines.append( QString( "  ... (%1 more records not shown)" ).arg( m_deckKeyword->size() - maxRecordsToShow ) );
    }

    // Add closing /
    lines.append( "/" );

    return lines.join( "\n" );
}
