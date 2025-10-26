#include "MainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafFactory.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafUiAppearanceSettings.h"

#include <QApplication>


int main( int argc, char* argv[] )
{
    // Configure UI appearance
    caf::UiAppearanceSettings::instance()->setAutoValueEditorColor( "moccasin" );

    auto appExitCode = 0;
    {
        QApplication app( argc, argv );

        // Create command feature manager singleton
        caf::CmdFeatureManager::createSingleton();

        MainWindow window;
        window.setWindowTitle( "Data Object Editor" );
        window.resize( 1200, 800 );
        window.show();

        appExitCode = app.exec();
    }

    // Cleanup singletons
    caf::CmdFeatureManager::deleteSingleton();
    caf::PdmDefaultObjectFactory::deleteSingleton();

    // Delete factory objects
    {
        auto factory = caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance();
        factory->deleteCreatorObjects();
    }
    {
        auto factory = caf::Factory<caf::CmdFeature, std::string>::instance();
        factory->deleteCreatorObjects();
    }

    return appExitCode;
}
