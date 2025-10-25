#pragma once

#include <QMainWindow>
#include <QStringList>

class QMenu;
class QAction;

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
    void buildTestModel();
    void releaseTestData();

    // Recent files management
    void        loadRecentFiles();
    void        saveRecentFiles();
    void        addRecentFile( const QString& filePath );
    void        updateRecentFilesMenu();
    QString     mostRecentFile() const;
    bool        importDataFile( const QString& filePath );

private slots:
    void slotNewProject();
    void slotImportDataFile();
    void slotOpenLastUsedDataFile();
    void slotOpenRecentFile();
    void slotSelectionChanged();
    void slotAbout();

private:
    static MainWindow* sm_mainWindowInstance;

    caf::PdmUiTreeView*     m_pdmUiTreeView;
    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmDocument*       m_project;

    // Recent files
    QStringList m_recentFiles;
    QMenu*      m_recentFilesMenu;
    QAction*    m_openLastUsedAction;
    static constexpr int MAX_RECENT_FILES = 10;
};
