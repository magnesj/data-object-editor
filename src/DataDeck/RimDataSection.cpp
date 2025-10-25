#include "RimDataSection.h"
#include "RimDataKeyword.h"

#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RimDataSection, "DataSection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSection::RimDataSection()
    : m_sectionType( SectionType::OTHER )
{
    CAF_PDM_InitObject( "Section", "", "", "" );

    CAF_PDM_InitField( &m_sectionName, "SectionName", QString( "" ), "Section", "", "", "" );
    m_sectionName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_keywordCount, "KeywordCount", 0, "Keyword Count", "", "", "" );
    m_keywordCount.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_keywords, "Keywords", "Keywords", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSection::~RimDataSection()
{
    m_keywords.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSection::setSectionType( SectionType type )
{
    m_sectionType = type;
    m_sectionName = sectionTypeToString( type );
    setUiName( m_sectionName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSection::setSectionName( const QString& name )
{
    m_sectionName = name;
    setUiName( m_sectionName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSection::addKeyword( RimDataKeyword* keyword )
{
    if ( keyword )
    {
        m_keywords.push_back( keyword );
        m_keywordCount = static_cast<int>( m_keywords.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataSection::sectionName() const
{
    return m_sectionName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSection::SectionType RimDataSection::sectionType() const
{
    return m_sectionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDataSection::keywordCount() const
{
    return m_keywordCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmChildArrayField<RimDataKeyword*>& RimDataSection::keywords() const
{
    return m_keywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataSection::sectionTypeToString( SectionType type )
{
    switch ( type )
    {
        case SectionType::RUNSPEC:  return "RUNSPEC";
        case SectionType::GRID:     return "GRID";
        case SectionType::EDIT:     return "EDIT";
        case SectionType::PROPS:    return "PROPS";
        case SectionType::REGIONS:  return "REGIONS";
        case SectionType::SOLUTION: return "SOLUTION";
        case SectionType::SUMMARY:  return "SUMMARY";
        case SectionType::SCHEDULE: return "SCHEDULE";
        case SectionType::OTHER:    return "OTHER";
        default:                    return "UNKNOWN";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSection::SectionType RimDataSection::stringToSectionType( const QString& str )
{
    if ( str == "RUNSPEC" )  return SectionType::RUNSPEC;
    if ( str == "GRID" )     return SectionType::GRID;
    if ( str == "EDIT" )     return SectionType::EDIT;
    if ( str == "PROPS" )    return SectionType::PROPS;
    if ( str == "REGIONS" )  return SectionType::REGIONS;
    if ( str == "SOLUTION" ) return SectionType::SOLUTION;
    if ( str == "SUMMARY" )  return SectionType::SUMMARY;
    if ( str == "SCHEDULE" ) return SectionType::SCHEDULE;

    return SectionType::OTHER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_sectionName );
    uiOrdering.add( &m_keywordCount );

    uiOrdering.skipRemainingFields( true );
}
