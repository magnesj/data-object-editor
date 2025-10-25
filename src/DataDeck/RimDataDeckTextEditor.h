#pragma once

#include <QPlainTextEdit>
#include <QWidget>

class RimDataDeck;
class DataFileSyntaxHighlighter;
class LineNumberArea;

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

signals:
    void modificationChanged( bool changed );

protected:
    void resizeEvent( QResizeEvent* event ) override;

private slots:
    void updateLineNumberAreaWidth( int newBlockCount );
    void highlightCurrentLine();
    void updateLineNumberArea( const QRect& rect, int dy );

private:
    RimDataDeck*                m_dataDeck;
    DataFileSyntaxHighlighter*  m_syntaxHighlighter;
    QWidget*                    m_lineNumberArea;
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
