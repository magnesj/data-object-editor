#include "MainWindow.h"

// AppFwk includes
#include "cafPdmDocument.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTreeView.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiObjectHandle.h"
#include "cafSelectionManager.h"

// DataDeck includes
#include "DataDeck/RimDataDeck.h"
#include "DataDeck/RimDataKeyword.h"
#include "DataDeck/RicImportDataDeckFeature.h"
#include "DataDeck/RimDataDeckTextEditor.h"
#include "DataDeck/KeywordHelpWidget.h"

// Qt includes
#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolBar>

// opm-common includes
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"
#include "opm/input/eclipse/Deck/Deck.hpp"

//==================================================================================================
/// Project Document - Root object for the data object editor project
//==================================================================================================
class ProjectDocument : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    ProjectDocument()
    {
        CAF_PDM_InitObject( "Data Object Editor Project", "", "", "" );
        CAF_PDM_InitFieldNoDefault( &m_dataDecks, "DataDecks", "DATA Files", "", "", "" );
    }

    caf::PdmChildArrayField<RimDataDeck*> m_dataDecks;
};

CAF_PDM_SOURCE_INIT( ProjectDocument, "ProjectDocument" );

//==================================================================================================
/// MainWindow implementation
//==================================================================================================

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

MainWindow::MainWindow()
    : m_pdmUiTreeView( nullptr )
    , m_pdmUiPropertyView( nullptr )
    , m_project( nullptr )
    , m_textEditor( nullptr )
    , m_keywordHelpWidget( nullptr )
    , m_updatingFromTree( false )
    , m_textEditorToolBar( nullptr )
    , m_syncTextToTreeAction( nullptr )
    , m_syncTreeToTextAction( nullptr )
    , m_recentFilesMenu( nullptr )
    , m_openLastUsedAction( nullptr )
{
    sm_mainWindowInstance = this;

    // Create text editor as central widget
    m_textEditor = new RimDataDeckTextEditor( this );
    setCentralWidget( m_textEditor );

    // Create dock panels
    createDockPanels();

    // Load recent files from settings
    loadRecentFiles();

    // Create actions and menus
    createActions();
    createMenus();
    createToolBar();

    // Create an empty project
    createEmptyProject();

    // Auto-open last used DATA file
    QString lastFile = mostRecentFile();
    if ( !lastFile.isEmpty() && QFileInfo::exists( lastFile ) )
    {
        importDataFile( lastFile );
    }

    // Status bar
    statusBar()->showMessage( "Ready" );
}

MainWindow::~MainWindow()
{
    // Clear UI views before deleting objects to avoid CAF_ASSERT
    if ( m_pdmUiTreeView )
    {
        m_pdmUiTreeView->setPdmItem( nullptr );
    }

    if ( m_pdmUiPropertyView )
    {
        m_pdmUiPropertyView->showProperties( nullptr );
    }

    releaseProjectData();
    sm_mainWindowInstance = nullptr;
}

MainWindow* MainWindow::instance()
{
    return sm_mainWindowInstance;
}

void MainWindow::createActions()
{
    // Actions will be connected in createMenus()
}

void MainWindow::createDockPanels()
{
    // Create tree view dock
    QDockWidget* treeDock = nullptr;
    {
        QDockWidget* dockWidget = new QDockWidget( "Project Tree", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiTreeView = new caf::PdmUiTreeView( this );
        dockWidget->setWidget( m_pdmUiTreeView );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
        treeDock = dockWidget;
    }

    // Create property view dock (below tree view)
    {
        QDockWidget* dockWidget = new QDockWidget( "Properties", this );
        dockWidget->setObjectName( "propertiesPanel" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiPropertyView = new caf::PdmUiPropertyView( dockWidget );
        dockWidget->setWidget( m_pdmUiPropertyView );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );

        // Stack property view below tree view
        splitDockWidget( treeDock, dockWidget, Qt::Vertical );
    }

    // Create keyword help dock (right side)
    {
        QDockWidget* dockWidget = new QDockWidget( "Keyword Help", this );
        dockWidget->setObjectName( "keywordHelpPanel" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_keywordHelpWidget = new KeywordHelpWidget( dockWidget );
        dockWidget->setWidget( m_keywordHelpWidget );

        addDockWidget( Qt::RightDockWidgetArea, dockWidget );
        
        // Connect text editor to help widget
        m_textEditor->setKeywordHelpWidget( m_keywordHelpWidget );
    }

    // Connect text editor modification signal
    connect( m_textEditor, &RimDataDeckTextEditor::modificationChanged, this, &MainWindow::slotTextEditorModified );

    // Connect text editor cursor position changes to tree selection
    connect( m_textEditor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::slotTextCursorChanged );

    // Connect tree view selection to property view and text editor
    connect( m_pdmUiTreeView, SIGNAL( selectionChanged() ), this, SLOT( slotSelectionChanged() ) );
}

void MainWindow::createMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu( "&File" );

    QAction* newAction = new QAction( "&New", this );
    newAction->setShortcuts( QKeySequence::New );
    connect( newAction, &QAction::triggered, this, &MainWindow::slotNewProject );
    fileMenu->addAction( newAction );

    fileMenu->addSeparator();

    QAction* importDataAction = new QAction( "Import &DATA File...", this );
    connect( importDataAction, &QAction::triggered, this, &MainWindow::slotImportDataFile );
    fileMenu->addAction( importDataAction );

    // Open Last Used DATA File
    m_openLastUsedAction = new QAction( "Open &Last Used DATA File", this );
    m_openLastUsedAction->setEnabled( !mostRecentFile().isEmpty() );
    connect( m_openLastUsedAction, &QAction::triggered, this, &MainWindow::slotOpenLastUsedDataFile );
    fileMenu->addAction( m_openLastUsedAction );

    fileMenu->addSeparator();

    // Recent Files submenu
    m_recentFilesMenu = fileMenu->addMenu( "&Recent DATA Files" );
    updateRecentFilesMenu();

    fileMenu->addSeparator();

    QAction* exitAction = new QAction( "E&xit", this );
    exitAction->setShortcuts( QKeySequence::Quit );
    connect( exitAction, &QAction::triggered, this, &QWidget::close );
    fileMenu->addAction( exitAction );

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu( "&Help" );

    QAction* aboutAction = new QAction( "&About", this );
    connect( aboutAction, &QAction::triggered, this, &MainWindow::slotAbout );
    helpMenu->addAction( aboutAction );
}

void MainWindow::createToolBar()
{
    m_textEditorToolBar = addToolBar( "Text Editor Tools" );
    m_textEditorToolBar->setObjectName( "textEditorToolBar" );

    // Sync Tree to Text action
    m_syncTreeToTextAction = new QAction( "Sync to Text", this );
    m_syncTreeToTextAction->setToolTip( "Synchronize selected DATA file to text editor" );
    m_syncTreeToTextAction->setEnabled( false );
    connect( m_syncTreeToTextAction, &QAction::triggered, this, &MainWindow::slotSyncTreeToText );
    m_textEditorToolBar->addAction( m_syncTreeToTextAction );

    // Sync Text to Tree action
    m_syncTextToTreeAction = new QAction( "Sync to Tree", this );
    m_syncTextToTreeAction->setToolTip( "Parse text and update object tree" );
    m_syncTextToTreeAction->setEnabled( false );
    connect( m_syncTextToTreeAction, &QAction::triggered, this, &MainWindow::slotSyncTextToTree );
    m_textEditorToolBar->addAction( m_syncTextToTreeAction );
}

void MainWindow::createEmptyProject()
{
    // Clear UI views before deleting old project
    if ( m_pdmUiTreeView )
    {
        m_pdmUiTreeView->setPdmItem( nullptr );
    }

    if ( m_pdmUiPropertyView )
    {
        m_pdmUiPropertyView->showProperties( nullptr );
    }

    releaseProjectData();

    // Create an empty project document
    m_project = new ProjectDocument();

    // Set the document as root in tree view
    m_pdmUiTreeView->setPdmItem( m_project );

    // Update UI
    m_project->updateConnectedEditors();

    statusBar()->showMessage( "New project created" );
}

void MainWindow::releaseProjectData()
{
    if ( m_project )
    {
        delete m_project;
        m_project = nullptr;
    }
}

void MainWindow::slotNewProject()
{
    createEmptyProject();
}

void MainWindow::slotImportDataFile()
{
    if ( !m_project )
    {
        QMessageBox::warning( this, "Import DATA File", "No project loaded!" );
        return;
    }

    QString filePath = QFileDialog::getOpenFileName( this,
                                                      "Import Eclipse DATA File",
                                                      "",
                                                      "Eclipse DATA Files (*.DATA *.data);;All Files (*.*)" );

    if ( !filePath.isEmpty() )
    {
        importDataFile( filePath );
    }
}

void MainWindow::slotOpenLastUsedDataFile()
{
    if ( !m_project )
    {
        QMessageBox::warning( this, "Open Last Used DATA File", "No project loaded!" );
        return;
    }

    QString lastFile = mostRecentFile();
    if ( !lastFile.isEmpty() )
    {
        importDataFile( lastFile );
    }
}

void MainWindow::slotOpenRecentFile()
{
    if ( !m_project )
    {
        QMessageBox::warning( this, "Open Recent File", "No project loaded!" );
        return;
    }

    QAction* action = qobject_cast<QAction*>( sender() );
    if ( action )
    {
        QString filePath = action->data().toString();
        importDataFile( filePath );
    }
}

void MainWindow::slotSelectionChanged()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems( selection );

    caf::PdmObjectHandle* obj = nullptr;

    if ( !selection.empty() )
    {
        caf::PdmUiObjectHandle* pdmUiObj = dynamic_cast<caf::PdmUiObjectHandle*>( selection[0] );
        if ( pdmUiObj )
        {
            obj = pdmUiObj->objectHandle();
        }
    }

    m_pdmUiPropertyView->showProperties( obj );

    // Update text editor first
    updateTextEditor();
    
    // Then synchronize text editor selection with tree selection
    if ( m_textEditor && obj && !m_updatingFromTree )
    {
        RimDataKeyword* keyword = dynamic_cast<RimDataKeyword*>( obj );
        if ( keyword )
        {
            int startLine = keyword->startLine();
            int endLine = keyword->endLine();
            
            if ( startLine >= 0 && endLine >= 0 )
            {
                m_updatingFromTree = true;
                highlightTextRange( startLine, endLine );
                m_updatingFromTree = false;
                
                // Show brief status message with position info
                statusBar()->showMessage( QString( "Selected: %1 (lines %2-%3)" )
                                         .arg( keyword->keywordName() )
                                         .arg( startLine )
                                         .arg( endLine ), 2000 );
            }
            else
            {
                statusBar()->showMessage( "No text position available for this item", 2000 );
            }
        }
    }
}

void MainWindow::slotAbout()
{
    QMessageBox::about( this,
                        "About Data Object Editor",
                        "Data Object Editor\n\n"
                        "An application for viewing and editing Eclipse DATA files using:\n"
                        "- AppFwk (Application Framework)\n"
                        "- opm-common\n\n"
                        "Built with Qt6 and C++23" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::loadRecentFiles()
{
    QSettings settings( "Ceetron", "DataObjectEditor" );
    m_recentFiles = settings.value( "recentFiles" ).toStringList();

    // Remove files that no longer exist
    QStringList validFiles;
    for ( const QString& file : m_recentFiles )
    {
        if ( QFileInfo::exists( file ) )
        {
            validFiles.append( file );
        }
    }
    m_recentFiles = validFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::saveRecentFiles()
{
    QSettings settings( "Ceetron", "DataObjectEditor" );
    settings.setValue( "recentFiles", m_recentFiles );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::addRecentFile( const QString& filePath )
{
    // Remove if already in list
    m_recentFiles.removeAll( filePath );

    // Add to front
    m_recentFiles.prepend( filePath );

    // Keep only MAX_RECENT_FILES
    while ( m_recentFiles.size() > MAX_RECENT_FILES )
    {
        m_recentFiles.removeLast();
    }

    // Save and update menu
    saveRecentFiles();
    updateRecentFilesMenu();

    // Enable "Open Last Used" action
    if ( m_openLastUsedAction )
    {
        m_openLastUsedAction->setEnabled( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::updateRecentFilesMenu()
{
    if ( !m_recentFilesMenu )
    {
        return;
    }

    m_recentFilesMenu->clear();

    if ( m_recentFiles.isEmpty() )
    {
        QAction* emptyAction = m_recentFilesMenu->addAction( "(No recent files)" );
        emptyAction->setEnabled( false );
        return;
    }

    for ( int i = 0; i < m_recentFiles.size(); ++i )
    {
        const QString& filePath = m_recentFiles[i];
        QFileInfo      fileInfo( filePath );

        QString actionText = QString( "&%1 %2" ).arg( i + 1 ).arg( fileInfo.fileName() );
        QAction* action    = m_recentFilesMenu->addAction( actionText );
        action->setData( filePath );
        action->setToolTip( filePath );
        action->setStatusTip( filePath );
        connect( action, &QAction::triggered, this, &MainWindow::slotOpenRecentFile );
    }

    m_recentFilesMenu->addSeparator();

    QAction* clearAction = m_recentFilesMenu->addAction( "Clear Recent Files" );
    connect( clearAction, &QAction::triggered, this, [this]() {
        m_recentFiles.clear();
        saveRecentFiles();
        updateRecentFilesMenu();
        if ( m_openLastUsedAction )
        {
            m_openLastUsedAction->setEnabled( false );
        }
    } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString MainWindow::mostRecentFile() const
{
    if ( m_recentFiles.isEmpty() )
    {
        return QString();
    }
    return m_recentFiles.first();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool MainWindow::importDataFile( const QString& filePath )
{
    if ( !m_project )
    {
        return false;
    }

    RimDataDeck* dataDeck = RicImportDataDeckFeature::importDataFile( filePath );

    if ( dataDeck )
    {
        ProjectDocument* doc = dynamic_cast<ProjectDocument*>( m_project );
        if ( doc )
        {
            doc->m_dataDecks.push_back( dataDeck );
            m_project->updateConnectedEditors();

            // Add to recent files
            addRecentFile( filePath );

            statusBar()->showMessage( QString( "Imported: %1 with %2 keywords" )
                                          .arg( dataDeck->filePath() )
                                          .arg( dataDeck->keywordCount() ) );
            return true;
        }
    }
    else
    {
        QMessageBox::critical( this,
                               "Import Failed",
                               QString( "Failed to import DATA file:\n%1" ).arg( filePath ) );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::updateTextEditor()
{
    RimDataDeck* dataDeck = getCurrentDataDeck();

    if ( dataDeck && m_textEditor )
    {
        m_textEditor->setDataDeck( dataDeck );
        m_syncTreeToTextAction->setEnabled( true );
        m_syncTextToTreeAction->setEnabled( false ); // Only enable after modifications
    }
    else
    {
        if ( m_textEditor )
        {
            m_textEditor->setDataDeck( nullptr );
        }
        m_syncTreeToTextAction->setEnabled( false );
        m_syncTextToTreeAction->setEnabled( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeck* MainWindow::getCurrentDataDeck()
{
    if ( !m_pdmUiTreeView )
    {
        return nullptr;
    }

    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems( selection );

    if ( selection.empty() )
    {
        return nullptr;
    }

    // Check if selected item is a RimDataDeck
    caf::PdmUiObjectHandle* pdmUiObj = dynamic_cast<caf::PdmUiObjectHandle*>( selection[0] );
    if ( pdmUiObj )
    {
        RimDataDeck* dataDeck = dynamic_cast<RimDataDeck*>( pdmUiObj->objectHandle() );
        if ( dataDeck )
        {
            return dataDeck;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSyncTreeToText()
{
    if ( !m_textEditor )
    {
        return;
    }

    RimDataDeck* dataDeck = getCurrentDataDeck();
    if ( !dataDeck )
    {
        statusBar()->showMessage( "No DATA file selected", 3000 );
        return;
    }

    // Reload text from deck
    m_textEditor->loadFromDeck();
    statusBar()->showMessage( "Synchronized tree to text editor", 3000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSyncTextToTree()
{
    if ( !m_textEditor )
    {
        return;
    }

    RimDataDeck* dataDeck = getCurrentDataDeck();
    if ( !dataDeck )
    {
        statusBar()->showMessage( "No DATA file selected", 3000 );
        return;
    }

    try
    {
        // Get text from editor
        QString text = m_textEditor->toPlainText();

        // Write to temporary file
        QString tempFilePath = dataDeck->filePath() + ".temp";
        QFile   tempFile( tempFilePath );
        if ( !tempFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            QMessageBox::critical( this, "Sync Error", "Failed to create temporary file for parsing" );
            return;
        }

        QTextStream out( &tempFile );
        out << text;
        tempFile.close();

        // Parse the text
        Opm::Parser       parser;
        Opm::ParseContext parseContext;
        parseContext.update( Opm::InputErrorAction::WARN );

        auto newDeck = std::make_shared<Opm::Deck>( parser.parseFile( tempFilePath.toStdString(), parseContext ) );

        // Delete temporary file
        QFile::remove( tempFilePath );

        // Update the data deck
        if ( dataDeck->updateFromDeck( newDeck ) )
        {
            // Mark as unmodified
            m_textEditor->document()->setModified( false );
            m_syncTextToTreeAction->setEnabled( false );

            statusBar()->showMessage( "Synchronized text to tree successfully", 3000 );
        }
        else
        {
            QMessageBox::critical( this, "Sync Error", "Failed to update DATA deck from text" );
        }
    }
    catch ( const std::exception& e )
    {
        QMessageBox::critical( this,
                               "Parse Error",
                               QString( "Failed to parse text:\n%1" ).arg( e.what() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotTextEditorModified( bool modified )
{
    // Enable sync to tree button when text is modified
    if ( m_syncTextToTreeAction )
    {
        m_syncTextToTreeAction->setEnabled( modified && getCurrentDataDeck() != nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::highlightTextRange( int startLine, int endLine )
{
    if ( !m_textEditor )
    {
        return;
    }

    // Convert line numbers to text positions (lines are 1-based, but QTextEdit is 0-based)
    QTextDocument* doc = m_textEditor->document();
    QTextBlock startBlock = doc->findBlockByLineNumber( startLine - 1 );
    QTextBlock endBlock = doc->findBlockByLineNumber( endLine - 1 );

    if ( !startBlock.isValid() || !endBlock.isValid() )
    {
        statusBar()->showMessage( QString( "Could not find lines %1-%2 in text" ).arg( startLine ).arg( endLine ), 2000 );
        return;
    }

    // Create text cursor and select the range
    QTextCursor cursor = m_textEditor->textCursor();
    cursor.setPosition( startBlock.position() );
    cursor.setPosition( endBlock.position() + endBlock.length() - 1, QTextCursor::KeepAnchor );

    // Set the cursor (this will highlight the selection)
    m_textEditor->setTextCursor( cursor );

    // Scroll to make the selection visible
    m_textEditor->ensureCursorVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::selectObjectAtTextPosition( int lineNumber )
{
    RimDataDeck* dataDeck = getCurrentDataDeck();
    if ( !dataDeck )
    {
        return;
    }

    // Find the keyword at this line
    RimDataKeyword* keyword = dataDeck->findKeywordAtLine( lineNumber );
    if ( !keyword )
    {
        return;
    }

    // Select the keyword in the tree
    m_pdmUiTreeView->selectAsCurrentItem( keyword );

    // Update property view
    m_pdmUiPropertyView->showProperties( keyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotTextCursorChanged()
{
    if ( !m_textEditor || m_updatingFromTree )
    {
        return;
    }

    // Get current cursor position
    QTextCursor cursor = m_textEditor->textCursor();
    QTextBlock block = cursor.block();
    int lineNumber = block.blockNumber() + 1; // Convert from 0-based to 1-based

    // Select corresponding object in tree (but don't highlight text to avoid recursion)
    selectObjectAtTextPosition( lineNumber );
}
