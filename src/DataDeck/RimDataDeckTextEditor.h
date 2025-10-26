#pragma once

#include <QPlainTextEdit>
#include <QWidget>
#include <QTimer>

class RimDataDeck;
class DataFileSyntaxHighlighter;
class DataFileCompleter;
class KeywordHelpWidget;
class LineNumberArea;
class RimDataKeyword; // Forward declaration for RimDataKeyword

enum ColumnType { String, Integer, Double };

struct ColumnInfo {
    ColumnType type = String;
    int maxLength = 0; // Max length for strings and integers
    int maxPreDecimal = 0; // Max length before decimal for doubles
    int maxPostDecimal = 0; // Max length after decimal for doubles
};

//==================================================================================================
/// Text editor for Eclipse DATA files with syntax highlighting and line numbers
//==================================================================================================
class RimDataDeckTextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit RimDataDeckTextEditor( QWidget* parent = nullptr );
    ~RimDataDeckTextEditor() override;

    void setDataDeck( RimDataDeck* dataDeck );
    RimDataDeck* dataDeck() const { return m_dataDeck; }

    void loadFromDeck();
    bool hasUnsavedChanges() const;

    void lineNumberAreaPaintEvent( QPaintEvent* event );
    int  lineNumberAreaWidth();

    void setKeywordHelpWidget( KeywordHelpWidget* helpWidget );
    void alignColumnsForKeyword( RimDataKeyword* keyword ); // New method

signals:
    void modificationChanged( bool changed );

protected:
    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;

private slots:
    void updateLineNumberAreaWidth( int newBlockCount );
    void highlightCurrentLine();
    void updateLineNumberArea( const QRect& rect, int dy );
    void onCursorPositionChanged();
    void updateKeywordHelp();

private:
    void setupCompleter();
    void insertCompletion( const QString& completion );
    QString textUnderCursor() const;
    
    RimDataDeck*                m_dataDeck;
    DataFileSyntaxHighlighter*  m_syntaxHighlighter;
    DataFileCompleter*          m_completer;
    KeywordHelpWidget*          m_helpWidget;
    QWidget*                    m_lineNumberArea;
    QTimer*                     m_helpUpdateTimer;
};

//==================================================================================================
/// Widget for displaying line numbers in the editor
//==================================================================================================
class LineNumberArea : public QWidget
{
public:
    LineNumberArea( RimDataDeckTextEditor* editor )
        : QWidget( editor )
        , m_textEditor( editor )
    {
    }

    QSize sizeHint() const override { return QSize( m_textEditor->lineNumberAreaWidth(), 0 ); }

protected:
    void paintEvent( QPaintEvent* event ) override { m_textEditor->lineNumberAreaPaintEvent( event ); }

private:
    RimDataDeckTextEditor* m_textEditor;
};
