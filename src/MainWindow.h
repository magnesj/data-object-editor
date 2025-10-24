#pragma once

#include <QMainWindow>

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

private slots:
    void slotLoadProject();
    void slotSaveProject();
    void slotNewProject();
    void slotAbout();

private:
    static MainWindow* sm_mainWindowInstance;

    caf::PdmUiTreeView*     m_pdmUiTreeView;
    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmDocument*       m_project;
};
