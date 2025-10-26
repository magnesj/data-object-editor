#include "RimIncludeKeyword.h"
#include "RimIncludeFile.h"

#include "cafPdmUiOrdering.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Deck/DeckItem.hpp"

#include <QDebug>

CAF_PDM_SOURCE_INIT(RimIncludeKeyword, "IncludeKeyword");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIncludeKeyword::RimIncludeKeyword()
{
    CAF_PDM_InitObject("INCLUDE Keyword", ":/File16x16.png");

    CAF_PDM_InitField(&m_includePath, "IncludePath", QString(), "Include Path");
    m_includePath.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_isResolved, "IsResolved", false, "Resolved");
    m_isResolved.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_includeFile, "IncludeFile", "Include File");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIncludeKeyword::~RimIncludeKeyword()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeKeyword::setIncludeFile(RimIncludeFile* includeFile)
{
    m_includeFile = includeFile;
    m_isResolved = (includeFile != nullptr && includeFile->fileExists());
    
    if (includeFile)
    {
        m_includePath = includeFile->includePath();
        
        // Update UI name to show status
        QString uiName = "INCLUDE " + includeFile->fileName();
        if (!includeFile->fileExists())
        {
            uiName += " (Missing)";
            setUiIconFromResourceString(":/Warning16x16.png");
        }
        else
        {
            uiName += " (Available)";
            setUiIconFromResourceString(":/File16x16.png");
        }
        setUiName(uiName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIncludeFile* RimIncludeKeyword::includeFile() const
{
    return m_includeFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimIncludeKeyword::includePath() const
{
    return m_includePath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIncludeKeyword::isIncludeResolved() const
{
    return m_isResolved;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeKeyword::setDeckKeyword(const Opm::DeckKeyword* deckKeyword)
{
    // Call parent implementation first
    RimDataKeyword::setDeckKeyword(deckKeyword);
    
    // Extract include path from the keyword
    extractIncludePathFromKeyword();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeKeyword::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    // Call parent implementation first
    RimDataKeyword::defineUiOrdering(uiConfigName, uiOrdering);
    
    // Add INCLUDE-specific fields
    uiOrdering.add(&m_includePath);
    uiOrdering.add(&m_isResolved);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIncludeKeyword::extractIncludePathFromKeyword()
{
    const Opm::DeckKeyword* deckKw = deckKeyword();
    if (!deckKw)
    {
        qDebug() << "RimIncludeKeyword::extractIncludePathFromKeyword - No deck keyword";
        return;
    }

    qDebug() << "RimIncludeKeyword::extractIncludePathFromKeyword - Processing keyword with" << deckKw->size() << "records";

    // INCLUDE keyword should have one record with one string item (the file path)
    if (deckKw->size() > 0)
    {
        const auto& record = deckKw->getRecord(0);
        qDebug() << "Record has" << record.size() << "items";
        if (record.size() > 0)
        {
            const auto& item = record.getItem(0);
            if (item.hasValue(0))
            {
                QString path = QString::fromStdString(item.getTrimmedString(0));
                m_includePath = path;
                qDebug() << "Extracted include path:" << path;
            }
            else
            {
                qDebug() << "Item has no value";
            }
        }
    }
    else
    {
        qDebug() << "No records in INCLUDE keyword";
    }
}