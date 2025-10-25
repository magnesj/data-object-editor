#include "RicImportDataDeckFeature.h"
#include "RimDataDeck.h"

#include <QFileDialog>
#include <QMessageBox>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeck* RicImportDataDeckFeature::importDataFile( const QString& filePath )
{
    if ( filePath.isEmpty() )
    {
        return nullptr;
    }

    RimDataDeck* dataDeck = new RimDataDeck();

    if ( dataDeck->loadFromFile( filePath ) )
    {
        return dataDeck;
    }
    else
    {
        delete dataDeck;
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataDeck* RicImportDataDeckFeature::showImportDialog( QWidget* parent )
{
    QString filePath = QFileDialog::getOpenFileName( parent,
                                                      "Import Eclipse DATA File",
                                                      "",
                                                      "Eclipse DATA Files (*.DATA *.data);;All Files (*.*)" );

    if ( filePath.isEmpty() )
    {
        return nullptr;
    }

    RimDataDeck* dataDeck = importDataFile( filePath );

    if ( !dataDeck )
    {
        QMessageBox::critical( parent,
                               "Import Failed",
                               QString( "Failed to parse DATA file:\n%1" ).arg( filePath ) );
    }

    return dataDeck;
}
