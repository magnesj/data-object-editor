#include "RimDataDeck.h"
#include "RimDataSection.h"
#include "RimDataKeyword.h"
#include "RimDataItem.h"
#include "RimIncludeFile.h"
#include "RimIncludeKeyword.h"

#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"

#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
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

    CAF_PDM_InitField( &m_basePath, "BasePath", QString( "" ), "Base Path", "", "", "" );
    m_basePath.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_sections, "Sections", "Sections", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_includeFiles, "IncludeFiles", "Include Files", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeck::~RimDataDeck()
{
    m_sections.deleteChildren();
    m_includeFiles.deleteChildren();
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
    m_basePath = fileInfo.absolutePath();

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
    
    // Resolve include file references by parsing the raw file
    resolveIncludesFromRawFile();
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
    
    // Add include files section if there are any
    if ( !m_includeFiles.empty() )
    {
        caf::PdmUiGroup* includeGroup = uiOrdering.addNewGroup( "Include Files" );
        includeGroup->add( &m_includeFiles );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    // Add sections first
    uiTreeOrdering.add( &m_sections );
    
    // Always add include files - the UI framework will handle empty arrays
    uiTreeOrdering.add( &m_includeFiles );
    
    uiTreeOrdering.skipRemainingChildren( true );
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

    // First pass: calculate line positions for each keyword
    calculateTextPositions();

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
            
            // Create a keyword object for the section delimiter itself
            RimDataKeyword* sectionKeyword = new RimDataKeyword();
            sectionKeyword->setDeckKeyword( &keyword );
            
            // Set text position from our calculated positions
            if ( m_keywordPositions.contains( i ) )
            {
                const auto& pos = m_keywordPositions[i];
                sectionKeyword->setTextPosition( pos.first, pos.second );
            }
            
            currentSection->addKeyword( sectionKeyword );
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

            // Create keyword wrapper - use specialized class for INCLUDE keywords
            RimDataKeyword* dataKeyword = nullptr;
            if ( keywordName == "INCLUDE" )
            {
                qDebug() << "Creating RimIncludeKeyword for INCLUDE at index" << i;
                dataKeyword = new RimIncludeKeyword();
            }
            else
            {
                dataKeyword = new RimDataKeyword();
            }
            dataKeyword->setDeckKeyword( &keyword );
            
            // Set text position from our calculated positions
            if ( m_keywordPositions.contains( i ) )
            {
                const auto& pos = m_keywordPositions[i];
                dataKeyword->setTextPosition( pos.first, pos.second );
            }
            
            currentSection->addKeyword( dataKeyword );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataDeck::updateFromDeck( std::shared_ptr<Opm::Deck> deck )
{
    if ( !deck )
    {
        return false;
    }

    m_deck         = deck;
    m_keywordCount = static_cast<int>( m_deck->size() );

    // Clear existing sections
    m_sections.deleteChildren();

    // Rebuild from new deck
    buildSectionsFromDeck();

    // Update all connected editors
    updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataDeck::serializeToText() const
{
    if ( !m_deck )
    {
        return QString();
    }

    QStringList lines;

    // Try to read original file if it exists
    if ( QFileInfo::exists( m_filePath ) )
    {
        QFile file( m_filePath );
        if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            QTextStream in( &file );
            QString     content = in.readAll();
            file.close();
            return content;
        }
    }

    // Otherwise, serialize from deck structure
    for ( size_t i = 0; i < m_deck->size(); ++i )
    {
        const Opm::DeckKeyword& keyword     = ( *m_deck )[i];
        QString                 keywordName = QString::fromStdString( keyword.name() );

        // Check if this is a section keyword
        bool isSection = ( keywordName == "RUNSPEC" || keywordName == "GRID" || keywordName == "EDIT" ||
                           keywordName == "PROPS" || keywordName == "REGIONS" || keywordName == "SOLUTION" ||
                           keywordName == "SUMMARY" || keywordName == "SCHEDULE" );

        if ( isSection )
        {
            // Add blank line before section
            if ( !lines.isEmpty() )
            {
                lines.append( "" );
            }
            lines.append( keywordName );
            lines.append( "" );
        }
        else
        {
            // Regular keyword
            lines.append( keywordName );

            // Add records
            for ( size_t recIdx = 0; recIdx < keyword.size(); ++recIdx )
            {
                const auto& record = keyword.getRecord( recIdx );

                QStringList itemValues;
                for ( size_t itemIdx = 0; itemIdx < record.size(); ++itemIdx )
                {
                    const auto& item = record.getItem( itemIdx );

                    if ( !item.hasValue( 0 ) )
                    {
                        itemValues.append( "*" );
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
                        itemValues.append( "*" );
                    }
                }

                if ( !itemValues.isEmpty() )
                {
                    lines.append( "  " + itemValues.join( "  " ) + "  /" );
                }
            }

            lines.append( "/" );
            lines.append( "" );
        }
    }

    return lines.join( "\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::calculateTextPositions()
{
    m_keywordPositions.clear();
    
    if ( !m_deck )
    {
        return;
    }

    // Get the actual text content that will be displayed
    QString textContent = serializeToText();
    QStringList lines = textContent.split( '\n' );
    
    // Track which lines we've already used to avoid duplicate matches
    QSet<int> usedLines;
    
    // Find each keyword in the actual text content
    for ( size_t i = 0; i < m_deck->size(); ++i )
    {
        const Opm::DeckKeyword& keyword = (*m_deck)[i];
        QString keywordName = QString::fromStdString( keyword.name() );
        
        // Search for this keyword in the text lines (starting from unused lines)
        int startLine = -1;
        int endLine = -1;
        
        for ( int lineIdx = 0; lineIdx < lines.size(); ++lineIdx )
        {
            // Skip lines we've already assigned to previous keywords
            if ( usedLines.contains( lineIdx ) )
                continue;
                
            QString line = lines[lineIdx].trimmed();
            
            // Check if this line contains our keyword name
            if ( line == keywordName )
            {
                startLine = lineIdx + 1; // Convert to 1-based line numbers
                usedLines.insert( lineIdx );
                
                // For section keywords, the end is usually just the keyword line
                bool isSection = ( keywordName == "RUNSPEC" || keywordName == "GRID" || keywordName == "EDIT" ||
                                  keywordName == "PROPS" || keywordName == "REGIONS" || keywordName == "SOLUTION" ||
                                  keywordName == "SUMMARY" || keywordName == "SCHEDULE" );
                
                if ( isSection )
                {
                    endLine = startLine; // Section keywords are single line
                }
                else
                {
                    // For regular keywords, find the terminating "/"
                    endLine = startLine;
                    for ( int searchIdx = lineIdx + 1; searchIdx < lines.size(); ++searchIdx )
                    {
                        QString searchLine = lines[searchIdx].trimmed();
                        usedLines.insert( searchIdx );
                        
                        if ( searchLine == "/" )
                        {
                            endLine = searchIdx + 1; // Include the "/" line
                            break;
                        }
                        else if ( !searchLine.isEmpty() )
                        {
                            endLine = searchIdx + 1; // Update end line as we find content
                        }
                    }
                }
                break; // Found the keyword, move to next
            }
        }
        
        if ( startLine > 0 && endLine > 0 )
        {
            m_keywordPositions[i] = QPair<int, int>( startLine, endLine );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataKeyword* RimDataDeck::findKeywordAtLine( int lineNumber )
{
    for ( RimDataSection* section : m_sections )
    {
        for ( RimDataKeyword* keyword : section->keywords() )
        {
            if ( keyword->startLine() <= lineNumber && lineNumber <= keyword->endLine() )
            {
                return keyword;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::addIncludeFile( RimIncludeFile* includeFile )
{
    if ( includeFile )
    {
        m_includeFiles.push_back( includeFile );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<RimIncludeFile*> RimDataDeck::includeFiles() const
{
    QList<RimIncludeFile*> files;
    for ( RimIncludeFile* file : m_includeFiles )
    {
        files.append( file );
    }
    return files;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::resolveIncludes()
{
    // Clear existing include files
    m_includeFiles.deleteChildren();
    
    // Map to track include files by path to avoid duplicates
    QMap<QString, RimIncludeFile*> includeFileMap;
    
    // Search through all sections and keywords to find INCLUDE keywords
    int totalKeywords = 0;
    int includeKeywordCount = 0;
    int includeKeywordCastCount = 0;
    int includePathCount = 0;
    QStringList allKeywordNames;
    
    for ( RimDataSection* section : m_sections )
    {
        for ( RimDataKeyword* keyword : section->keywords() )
        {
            totalKeywords++;
            QString keywordName = keyword->keywordName();
            
            // Collect first 20 keyword names for debugging
            if (allKeywordNames.size() < 20)
            {
                allKeywordNames.append(keywordName);
            }
            
            if ( keywordName == "INCLUDE" )
            {
                includeKeywordCount++;
                RimIncludeKeyword* includeKeyword = dynamic_cast<RimIncludeKeyword*>( keyword );
                if ( includeKeyword )
                {
                    includeKeywordCastCount++;
                    QString includePath = includeKeyword->includePath();
                    if ( !includePath.isEmpty() )
                    {
                        includePathCount++;
                        // Create or reuse include file object
                        RimIncludeFile* includeFile = nullptr;
                        if ( includeFileMap.contains( includePath ) )
                        {
                            includeFile = includeFileMap[includePath];
                        }
                        else
                        {
                            includeFile = new RimIncludeFile();
                            includeFile->setIncludePath( includePath, m_basePath );
                            includeFile->updateFileStatus();
                            
                            // Try to load the content if the file exists
                            if ( includeFile->fileExists() )
                            {
                                includeFile->loadContent();
                            }
                            
                            includeFileMap[includePath] = includeFile;
                            addIncludeFile( includeFile );
                        }
                        
                        // Link the keyword to the include file
                        includeKeyword->setIncludeFile( includeFile );
                    }
                }
            }
        }
    }
    
    // Debug output to console - these will show in debug builds
    qDebug() << "INCLUDE Detection Stats:";
    qDebug() << "  Total keywords:" << totalKeywords;
    qDebug() << "  INCLUDE keywords found:" << includeKeywordCount;
    qDebug() << "  Successfully cast to RimIncludeKeyword:" << includeKeywordCastCount;
    qDebug() << "  Include paths extracted:" << includePathCount;
    qDebug() << "  Include files created:" << m_includeFiles.size();
    qDebug() << "  First 20 keywords:" << allKeywordNames.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataDeck::resolveIncludesFromRawFile()
{
    // Clear existing include files
    m_includeFiles.deleteChildren();
    
    if (m_filePath().isEmpty())
    {
        qDebug() << "No file path available for include detection";
        return;
    }
    
    QFile file(m_filePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Could not open file for include detection:" << m_filePath();
        return;
    }
    
    QTextStream in(&file);
    QStringList includePaths;
    int lineNumber = 0;
    
    qDebug() << "Scanning file for INCLUDE statements:" << m_filePath();
    
    while (!in.atEnd())
    {
        QString line = in.readLine();
        lineNumber++;
        
        // Remove comments
        int commentPos = line.indexOf("--");
        if (commentPos >= 0)
        {
            line = line.left(commentPos);
        }
        
        QString trimmedLine = line.trimmed();
        
        // Check if line is exactly "INCLUDE"
        if (trimmedLine.compare("INCLUDE", Qt::CaseInsensitive) == 0)
        {
            qDebug() << "Found INCLUDE keyword at line" << lineNumber;
            
            // Read the next line which should contain the file path
            if (!in.atEnd())
            {
                QString nextLine = in.readLine();
                lineNumber++;
                
                // Remove comments from next line
                int nextCommentPos = nextLine.indexOf("--");
                if (nextCommentPos >= 0)
                {
                    nextLine = nextLine.left(nextCommentPos);
                }
                
                QString trimmedNextLine = nextLine.trimmed();
                qDebug() << "  Next line" << lineNumber << ":" << trimmedNextLine;
                
                // Extract filename from patterns like: 'filename' / or "filename" /
                QRegularExpression pathRegex(R"(['\"]([^'\"]+)['\"])", QRegularExpression::CaseInsensitiveOption);
                QRegularExpressionMatch match = pathRegex.match(trimmedNextLine);
                
                if (match.hasMatch())
                {
                    QString includePath = match.captured(1).trimmed();
                    qDebug() << "  Extracted include path:" << includePath;
                    
                    if (!includePath.isEmpty() && !includePaths.contains(includePath))
                    {
                        includePaths.append(includePath);
                        qDebug() << "  Added unique include path:" << includePath;
                    }
                    else if (!includePath.isEmpty())
                    {
                        qDebug() << "  Path already exists:" << includePath;
                    }
                }
                else
                {
                    qDebug() << "  Could not extract path from:" << trimmedNextLine;
                }
            }
            else
            {
                qDebug() << "  No next line available after INCLUDE";
            }
        }
    }
    
    file.close();
    
    qDebug() << "Found" << includePaths.size() << "unique include paths";
    
    // Create RimIncludeFile objects for each include
    for (const QString& includePath : includePaths)
    {
        RimIncludeFile* includeFile = new RimIncludeFile();
        includeFile->setIncludePath(includePath, m_basePath());
        includeFile->updateFileStatus();
        
        // Try to load the content if the file exists
        if (includeFile->fileExists())
        {
            includeFile->loadContent();
        }
        
        addIncludeFile(includeFile);
        qDebug() << "Created include file for:" << includePath;
    }
    
    qDebug() << "Final include files count:" << m_includeFiles.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataDeck::basePath() const
{
    return m_basePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimDataDeck::findIncludeReferences() const
{
    QStringList includePaths;
    
    // Search through all sections and keywords
    for ( const RimDataSection* section : m_sections )
    {
        for ( const RimDataKeyword* keyword : section->keywords() )
        {
            if ( keyword->keywordName() == "INCLUDE" )
            {
                // Try to cast to RimIncludeKeyword to get the include path
                const RimIncludeKeyword* includeKeyword = dynamic_cast<const RimIncludeKeyword*>( keyword );
                if ( includeKeyword )
                {
                    QString path = includeKeyword->includePath();
                    if ( !path.isEmpty() && !includePaths.contains( path ) )
                    {
                        includePaths.append( path );
                    }
                }
            }
        }
    }
    
    return includePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataDeck::validateIncludePaths() const
{
    bool allValid = true;
    
    for ( const RimIncludeFile* includeFile : m_includeFiles )
    {
        if ( !includeFile->fileExists() )
        {
            allValid = false;
        }
    }
    
    return allValid;
}
