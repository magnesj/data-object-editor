#pragma once

#include <QObject>
#include <QString>

class RimDataDeck;

//==================================================================================================
/// Feature for importing Eclipse DATA files
//==================================================================================================
class RicImportDataDeckFeature : public QObject
{
    Q_OBJECT

public:
    static RimDataDeck* importDataFile( const QString& filePath );
    static RimDataDeck* showImportDialog( QWidget* parent = nullptr );
};
