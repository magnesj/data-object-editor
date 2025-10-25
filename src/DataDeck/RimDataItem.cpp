#include "RimDataItem.h"

#include "cafPdmUiOrdering.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"

CAF_PDM_SOURCE_INIT( RimDataItem, "DataItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataItem::RimDataItem()
    : m_deckItem( nullptr )
{
    CAF_PDM_InitObject( "Data Item", "", "", "" );

    CAF_PDM_InitField( &m_itemName, "ItemName", QString( "" ), "Name", "", "", "" );
    m_itemName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_dataType, "DataType", QString( "" ), "Type", "", "", "" );
    m_dataType.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value", "", "", "" );
    m_value.registerGetMethod( this, &RimDataItem::formatValue );
    m_value.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataItem::~RimDataItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataItem::setDeckItem( const Opm::DeckItem* deckItem )
{
    m_deckItem = deckItem;

    if ( m_deckItem )
    {
        m_itemName = QString::fromStdString( m_deckItem->name() );

        // Determine data type
        if ( m_deckItem->hasValue( 0 ) )
        {
            if ( m_deckItem->getType() == Opm::type_tag::integer )
            {
                m_dataType = "INT";
            }
            else if ( m_deckItem->getType() == Opm::type_tag::fdouble )
            {
                m_dataType = "DOUBLE";
            }
            else if ( m_deckItem->getType() == Opm::type_tag::string )
            {
                m_dataType = "STRING";
            }
            else
            {
                m_dataType = "UNKNOWN";
            }
        }
        else
        {
            m_dataType = "DEFAULTED";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataItem::itemName() const
{
    return m_itemName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataItem::dataType() const
{
    return m_dataType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataItem::valueAsString() const
{
    return formatValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataItem::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_itemName );
    uiOrdering.add( &m_dataType );
    uiOrdering.add( &m_value );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataItem::formatValue() const
{
    if ( !m_deckItem )
    {
        return QString( "" );
    }

    if ( !m_deckItem->hasValue( 0 ) )
    {
        return QString( "*" ); // Defaulted value
    }

    QString result;

    try
    {
        if ( m_deckItem->getType() == Opm::type_tag::integer )
        {
            if ( m_deckItem->data_size() == 1 )
            {
                result = QString::number( m_deckItem->get<int>( 0 ) );
            }
            else
            {
                QStringList values;
                for ( size_t i = 0; i < std::min( m_deckItem->data_size(), size_t( 10 ) ); ++i )
                {
                    values.append( QString::number( m_deckItem->get<int>( i ) ) );
                }
                if ( m_deckItem->data_size() > 10 )
                {
                    values.append( QString( "... (%1 values)" ).arg( m_deckItem->data_size() ) );
                }
                result = values.join( ", " );
            }
        }
        else if ( m_deckItem->getType() == Opm::type_tag::fdouble )
        {
            if ( m_deckItem->data_size() == 1 )
            {
                result = QString::number( m_deckItem->get<double>( 0 ), 'g', 10 );
            }
            else
            {
                QStringList values;
                for ( size_t i = 0; i < std::min( m_deckItem->data_size(), size_t( 10 ) ); ++i )
                {
                    values.append( QString::number( m_deckItem->get<double>( i ), 'g', 10 ) );
                }
                if ( m_deckItem->data_size() > 10 )
                {
                    values.append( QString( "... (%1 values)" ).arg( m_deckItem->data_size() ) );
                }
                result = values.join( ", " );
            }
        }
        else if ( m_deckItem->getType() == Opm::type_tag::string )
        {
            if ( m_deckItem->data_size() == 1 )
            {
                result = QString::fromStdString( m_deckItem->get<std::string>( 0 ) );
            }
            else
            {
                QStringList values;
                for ( size_t i = 0; i < std::min( m_deckItem->data_size(), size_t( 10 ) ); ++i )
                {
                    values.append( QString::fromStdString( m_deckItem->get<std::string>( i ) ) );
                }
                if ( m_deckItem->data_size() > 10 )
                {
                    values.append( QString( "... (%1 values)" ).arg( m_deckItem->data_size() ) );
                }
                result = values.join( ", " );
            }
        }
    }
    catch ( ... )
    {
        result = QString( "<error reading value>" );
    }

    return result;
}
