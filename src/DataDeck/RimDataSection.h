#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"

class RimDataKeyword;

//==================================================================================================
/// Represents a section in an Eclipse DATA file (RUNSPEC, GRID, PROPS, etc.)
//==================================================================================================
class RimDataSection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SectionType
    {
        RUNSPEC,
        GRID,
        EDIT,
        PROPS,
        REGIONS,
        SOLUTION,
        SUMMARY,
        SCHEDULE,
        OTHER
    };

public:
    RimDataSection();
    ~RimDataSection() override;

    void setSectionType( SectionType type );
    void setSectionName( const QString& name );
    void addKeyword( RimDataKeyword* keyword );

    QString         sectionName() const;
    SectionType     sectionType() const;
    int             keywordCount() const;
    const caf::PdmChildArrayField<RimDataKeyword*>& keywords() const;

    static QString  sectionTypeToString( SectionType type );
    static SectionType stringToSectionType( const QString& str );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<QString>                      m_sectionName;
    caf::PdmField<int>                          m_keywordCount;

    caf::PdmChildArrayField<RimDataKeyword*>    m_keywords;

    SectionType                                 m_sectionType;
};
