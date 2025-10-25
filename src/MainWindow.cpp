#include "MainWindow.h"

// AppFwk includes
#include "cafPdmDocument.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

// Qt includes
#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

//==================================================================================================
/// Demo PDM Document - Root object for the project
//==================================================================================================
class DemoDocument : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    DemoDocument()
    {
        CAF_PDM_InitObject( "Demo Document", "", "", "" );
        CAF_PDM_InitFieldNoDefault( &m_objects, "Objects", "Objects", "", "", "" );
    }

    caf::PdmChildArrayField<caf::PdmObjectHandle*> m_objects;
};

CAF_PDM_SOURCE_INIT( DemoDocument, "DemoDocument" );

//==================================================================================================
/// Demo PDM Object - Simple example object with various field types
//==================================================================================================
class DemoObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    DemoObject()
    {
        CAF_PDM_InitObject( "Demo Object", "", "", "" );

        CAF_PDM_InitField( &m_name, "Name", QString( "Default Name" ), "Name", "", "", "" );
        CAF_PDM_InitField( &m_value, "Value", 42.0, "Value", "", "", "" );
        CAF_PDM_InitField( &m_isActive, "IsActive", true, "Is Active", "", "", "" );
        CAF_PDM_InitField( &m_text, "Text", QString( "Sample text" ), "Description", "", "", "" );
    }

    caf::PdmField<QString> m_name;
    caf::PdmField<double>  m_value;
    caf::PdmField<bool>    m_isActive;
    caf::PdmField<QString> m_text;
};

CAF_PDM_SOURCE_INIT( DemoObject, "DemoObject" );

//==================================================================================================
/// MainWindow implementation
//==================================================================================================

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

MainWindow::MainWindow()
    : m_pdmUiTreeView( nullptr )
    , m_pdmUiPropertyView( nullptr )
    , m_project( nullptr )
{
    sm_mainWindowInstance = this;

    // Set up the central widget
    QWidget* centralWidget = new QWidget( this );
    setCentralWidget( centralWidget );

    // Create dock panels
    createDockPanels();

    // Create actions and menus
    createActions();
    createMenus();

    // Create a test model
    buildTestModel();

    // Status bar
    statusBar()->showMessage( "Ready" );
}

MainWindow::~MainWindow()
{
    releaseTestData();
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
    {
        QDockWidget* dockWidget = new QDockWidget( "Project Tree", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiTreeView = new caf::PdmUiTreeView( this );
        dockWidget->setWidget( m_pdmUiTreeView );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
    }

    // Create property view dock
    {
        QDockWidget* dockWidget = new QDockWidget( "Properties", this );
        dockWidget->setObjectName( "propertiesPanel" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiPropertyView = new caf::PdmUiPropertyView( dockWidget );
        dockWidget->setWidget( m_pdmUiPropertyView );

        addDockWidget( Qt::RightDockWidgetArea, dockWidget );
    }

    // Connect tree view selection to property view
    connect( m_pdmUiTreeView, SIGNAL( selectionChanged() ), this, SLOT( slotLoadProject() ) );
}

void MainWindow::createMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu( "&File" );

    QAction* newAction = new QAction( "&New", this );
    newAction->setShortcuts( QKeySequence::New );
    connect( newAction, &QAction::triggered, this, &MainWindow::slotNewProject );
    fileMenu->addAction( newAction );

    QAction* openAction = new QAction( "&Open...", this );
    openAction->setShortcuts( QKeySequence::Open );
    connect( openAction, &QAction::triggered, this, &MainWindow::slotLoadProject );
    fileMenu->addAction( openAction );

    QAction* saveAction = new QAction( "&Save...", this );
    saveAction->setShortcuts( QKeySequence::Save );
    connect( saveAction, &QAction::triggered, this, &MainWindow::slotSaveProject );
    fileMenu->addAction( saveAction );

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

void MainWindow::buildTestModel()
{
    releaseTestData();

    // Create the document
    m_project = new DemoDocument();

    // Create some demo objects
    DemoObject* obj1 = new DemoObject();
    obj1->m_name     = "Object 1";
    obj1->m_value    = 100.0;
    obj1->m_isActive = true;

    DemoObject* obj2 = new DemoObject();
    obj2->m_name     = "Object 2";
    obj2->m_value    = 200.0;
    obj2->m_isActive = false;

    DemoObject* obj3 = new DemoObject();
    obj3->m_name     = "Object 3";
    obj3->m_value    = 300.0;
    obj3->m_isActive = true;

    // Add objects to document
    DemoDocument* doc = dynamic_cast<DemoDocument*>( m_project );
    if ( doc )
    {
        doc->m_objects.push_back( obj1 );
        doc->m_objects.push_back( obj2 );
        doc->m_objects.push_back( obj3 );
    }

    // Set the document as root in tree view
    m_pdmUiTreeView->setPdmItem( m_project );

    // Update UI
    m_project->updateConnectedEditors();

    statusBar()->showMessage( "Test model created with 3 demo objects" );
}

void MainWindow::releaseTestData()
{
    if ( m_project )
    {
        delete m_project;
        m_project = nullptr;
    }
}

void MainWindow::slotNewProject()
{
    buildTestModel();
    statusBar()->showMessage( "New project created" );
}

void MainWindow::slotLoadProject()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Open Project", "", "XML Files (*.xml)" );

    if ( !fileName.isEmpty() )
    {
        releaseTestData();

        m_project = new DemoDocument();
        //m_project->readFile( fileName );

        m_pdmUiTreeView->setPdmItem( m_project );
        m_project->updateConnectedEditors();

        statusBar()->showMessage( QString( "Loaded: %1" ).arg( fileName ) );
    }
}

void MainWindow::slotSaveProject()
{
    if ( !m_project )
    {
        QMessageBox::warning( this, "Save Project", "No project to save!" );
        return;
    }

    QString fileName = QFileDialog::getSaveFileName( this, "Save Project", "", "XML Files (*.xml)" );

    if ( !fileName.isEmpty() )
    {
        //m_project->writeFile( fileName );
        statusBar()->showMessage( QString( "Saved: %1" ).arg( fileName ) );
    }
}

void MainWindow::slotAbout()
{
    QMessageBox::about( this,
                        "About Data Object Editor",
                        "Data Object Editor\n\n"
                        "A demonstration application using:\n"
                        "- AppFwk (Application Framework)\n"
                        "- opm-common\n\n"
                        "Built with Qt6 and C++23" );
}
