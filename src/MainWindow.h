#pragma once

#include <QMainWindow>
#include <QStringList>

class QMenu;
class QAction;
class QToolBar;
class RimDataDeckTextEditor;
class RimDataDeck;
class KeywordHelpWidget;

namespace caf
{
class PdmObjectHandle;
class PdmDocument;
class PdmUiTreeView;
class PdmUiPropertyView;
} // namespace caf

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    static MainWindow* instance();

private:
    void createActions();
    void createDockPanels();
    void createMenus();
    void createToolBar();
    void createEmptyProject();
    void releaseProjectData();

    // Recent files management
    void        loadRecentFiles();
    void        saveRecentFiles();
    void        addRecentFile( const QString& filePath );
    void        updateRecentFilesMenu();
    QString     mostRecentFile() const;
    bool        importDataFile( const QString& filePath );

    // Text editor synchronization
    void        updateTextEditor();
    RimDataDeck* getCurrentDataDeck();
    void        highlightTextRange( int startLine, int endLine );
    void        selectObjectAtTextPosition( int lineNumber );

private slots:
    void slotNewProject();
    void slotImportDataFile();
    void slotOpenLastUsedDataFile();
    void slotOpenRecentFile();
    void slotSelectionChanged();
    void slotAbout();

    // Text editor synchronization
    void slotSyncTextToTree();
    void slotSyncTreeToText();
    void slotTextEditorModified( bool modified );
    void slotTextCursorChanged();

private:
    static MainWindow* sm_mainWindowInstance;

    caf::PdmUiTreeView*     m_pdmUiTreeView;
    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmDocument*       m_project;

    // Text editor
    RimDataDeckTextEditor*  m_textEditor;
    KeywordHelpWidget*      m_keywordHelpWidget;
    QToolBar*               m_textEditorToolBar;
    QAction*                m_syncTextToTreeAction;
    QAction*                m_syncTreeToTextAction;

    // Recent files
    QStringList m_recentFiles;
    QMenu*      m_recentFilesMenu;
    QAction*    m_openLastUsedAction;
    static constexpr int MAX_RECENT_FILES = 10;

    // Synchronization state
    bool        m_updatingFromTree;
};
