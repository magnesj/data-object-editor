#include "RimDataDeck.h"
#include "RimDataSection.h"
#include "RimDataKeyword.h"

#include "cafPdmUiOrdering.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"

#include <QFileInfo>
#include <stdexcept>

CAF_PDM_SOURCE_INIT( RimDataDeck, "DataDeck" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeck::RimDataDeck()
{
    CAF_PDM_InitObject( "DATA File", "", "", "" );

    CAF_PDM_InitField( &m_filePath, "FilePath", QString( "" ), "File Path", "", "", "" );
    m_filePath.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_fileName, "FileName", QString( "" ), "File Name", "", "", "" );
    m_fileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_keywordCount, "KeywordCount", 0, "Total Keywords", "", "", "" );
    m_keywordCount.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_sections, "Sections", "Sections", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeck::~RimDataDeck()
{
    m_sections.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataDeck::loadFromFile( const QString& filePath )
{
    try
    {
        // Create parser with default configuration
        Opm::Parser parser;

        // Set up parse context to handle errors gracefully
        Opm::ParseContext parseContext;
        parseContext.update( Opm::InputErrorAction::WARN );

        // Parse the file
        auto deck = std::make_shared<Opm::Deck>( parser.parseFile( filePath.toStdString(), parseContext ) );

        // Store deck and build UI structure
        setDeck( deck, filePath );

        return true;
    }
    catch ( const std::exception& e )
    {
        // Error handling - could be improved with proper logging
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::setDeck( std::shared_ptr<Opm::Deck> deck, const QString& filePath )
{
    m_deck = deck;
    m_filePath = filePath;

    QFileInfo fileInfo( filePath );
    m_fileName = fileInfo.fileName();

    if ( m_deck )
    {
        m_keywordCount = static_cast<int>( m_deck->size() );
    }
    else
    {
        m_keywordCount = 0;
    }

    // Update UI name to show file name
    setUiName( m_fileName );

    // Build section structure
    buildSectionsFromDeck();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataDeck::filePath() const
{
    return m_filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDataDeck::keywordCount() const
{
    return m_keywordCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<Opm::Deck> RimDataDeck::deck() const
{
    return m_deck;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_fileName );
    uiOrdering.add( &m_filePath );
    uiOrdering.add( &m_keywordCount );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::buildSectionsFromDeck()
{
    m_sections.deleteChildren();

    if ( !m_deck )
    {
        return;
    }

    // Create sections for organizing keywords
    RimDataSection* currentSection = nullptr;
    RimDataSection::SectionType currentSectionType = RimDataSection::SectionType::OTHER;

    for ( size_t i = 0; i < m_deck->size(); ++i )
    {
        const Opm::DeckKeyword& keyword = (*m_deck)[i];
        QString keywordName = QString::fromStdString( keyword.name() );

        // Check if this is a section keyword
        RimDataSection::SectionType newSectionType = RimDataSection::stringToSectionType( keywordName );

        if ( newSectionType != RimDataSection::SectionType::OTHER )
        {
            // This is a section delimiter keyword
            currentSectionType = newSectionType;
            currentSection = new RimDataSection();
            currentSection->setSectionType( currentSectionType );
            m_sections.push_back( currentSection );
        }
        else
        {
            // Regular keyword - add to current section
            if ( !currentSection )
            {
                // No section defined yet, create an "Other" section
                currentSection = new RimDataSection();
                currentSection->setSectionType( RimDataSection::SectionType::OTHER );
                currentSection->setSectionName( "Pre-RUNSPEC" );
                m_sections.push_back( currentSection );
            }

            // Create keyword wrapper
            RimDataKeyword* dataKeyword = new RimDataKeyword();
            dataKeyword->setDeckKeyword( &keyword );
            currentSection->addKeyword( dataKeyword );
        }
    }
}
