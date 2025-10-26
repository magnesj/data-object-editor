#include "KeywordHelpWidget.h"
#include "KeywordDatabase.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QFont>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
KeywordHelpWidget::KeywordHelpWidget(QWidget* parent)
    : QWidget(parent)
    , m_keywordDatabase(KeywordDatabase::instance())
{
    setupUI();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordHelpWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(4);
    
    // Title label
    m_titleLabel = new QLabel("Keyword Help", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("QLabel { color: #3794ff; }");
    m_layout->addWidget(m_titleLabel);
    
    // Help text area
    m_helpText = new QTextEdit(this);
    m_helpText->setReadOnly(true);
    //m_helpText->setFont(QFont("Cascadia Mono", 9));
    m_helpText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_helpText->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_layout->addWidget(m_helpText, 1); // Add with stretch factor 1 to use all available space
    
    // Initialize with empty help
    clearHelp();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordHelpWidget::showKeywordHelp(const QString& keyword, const QString& currentSection)
{
    if (keyword.isEmpty())
    {
        clearHelp();
        return;
    }
    
    if (m_keywordDatabase->hasKeyword(keyword))
    {
        KeywordInfo info = m_keywordDatabase->getKeywordInfo(keyword);
        formatKeywordInfo(info, currentSection);
    }
    else if (m_keywordDatabase->isSection(keyword))
    {
        m_titleLabel->setText(QString("Section: %1").arg(keyword));
        m_helpText->setHtml(QString("<b>%1</b><br><br>"
                                   "This is a section keyword that defines the start of the %2 section.<br><br>"
                                   "Keywords in this section control %3.")
                           .arg(keyword)
                           .arg(keyword)
                           .arg(getSectionDescription(keyword)));
    }
    else
    {
        m_titleLabel->setText("Unknown Keyword");
        m_helpText->setHtml(QString("<i>Keyword '%1' not found in database.</i><br><br>"
                                   "This might be a user-defined keyword or a typo.")
                           .arg(keyword));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordHelpWidget::clearHelp()
{
    m_titleLabel->setText("Keyword Help");
    m_helpText->setHtml("<i>Place cursor on a keyword to see help information.</i>");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void KeywordHelpWidget::formatKeywordInfo(const KeywordInfo& info, const QString& currentSection)
{
    m_titleLabel->setText(info.name);
    
    QString html = QString("<b>%1</b><br><br>").arg(info.name);
    
    // Valid sections
    if (!info.validSections.isEmpty())
    {
        html += "<b>Valid sections:</b> ";
        QStringList sections = info.validSections;
        
        // Highlight current section if it matches
        for (int i = 0; i < sections.size(); ++i)
        {
            if (!currentSection.isEmpty() && 
                sections[i].compare(currentSection, Qt::CaseInsensitive) == 0)
            {
                sections[i] = QString("<span style='color: #00aa00; font-weight: bold;'>%1</span>").arg(sections[i]);
            }
        }
        
        html += sections.join(", ") + "<br><br>";
        
        // Check if keyword is valid in current section
        if (!currentSection.isEmpty() && !info.isValidInSection(currentSection))
        {
            html += QString("<span style='color: #ff6666; font-weight: bold;'>⚠ Warning: %1 is not valid in section %2</span><br><br>")
                   .arg(info.name).arg(currentSection);
        }
    }
    
    // Data type
    if (!info.valueType.isEmpty())
    {
        html += QString("<b>Data type:</b> %1<br><br>").arg(info.valueType);
    }
    
    // Description
    if (!info.description.isEmpty())
    {
        html += QString("<b>Description:</b> %1<br><br>").arg(info.description);
    }

    // Parameters with detailed column information
    if (!info.parameterNames.isEmpty())
    {
        html += "<b>Parameters (Columns):</b><br>";
        html += "<table style='border-collapse: collapse; margin: 5px 0; background-color: white; color: black;'>";
        html += "<tr style='background-color: #e0e0e0; font-weight: bold; color: black;'>";
        html += "<td style='padding: 4px 8px; border: 1px solid #888; color: black;'>Col</td>";
        html += "<td style='padding: 4px 8px; border: 1px solid #888; color: black;'>Parameter</td>";
        html += "<td style='padding: 4px 8px; border: 1px solid #888; color: black;'>Type</td>";
        html += "<td style='padding: 4px 8px; border: 1px solid #888; color: black;'>Description</td>";
        html += "</tr>";
        
        for (int i = 0; i < info.parameterNames.size(); ++i)
        {
            QString paramName = info.parameterNames[i];
            QString paramType = (i < info.parameterTypes.size()) ? info.parameterTypes[i] : "UNKNOWN";
            QString paramDesc = (i < info.parameterDescriptions.size()) ? info.parameterDescriptions[i] : "";
            
            QString rowBg = (i % 2 == 0) ? "#f5f5f5" : "#ffffff";
            html += QString("<tr style='background-color: %1; color: black;'>").arg(rowBg);
            html += QString("<td style='padding: 4px 8px; border: 1px solid #888; text-align: center; font-weight: bold; color: black;'>%1</td>").arg(i + 1);
            html += QString("<td style='padding: 4px 8px; border: 1px solid #888; font-family: monospace; color: black;'>%1</td>").arg(paramName);
            html += QString("<td style='padding: 4px 8px; border: 1px solid #888; color: #333;'>%1</td>").arg(paramType);
            html += QString("<td style='padding: 4px 8px; border: 1px solid #888; color: black;'>%1</td>").arg(paramDesc);
            html += "</tr>";
        }
        html += "</table><br>";
    }
    
    // Size information
    if (info.hasSize && !info.sizeKeyword.isEmpty())
    {
        html += QString("<b>Size:</b> Defined by %1<br><br>").arg(info.sizeKeyword);
    }
    
    m_helpText->setHtml(html);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString KeywordHelpWidget::getSectionDescription(const QString& section) const
{
    static QMap<QString, QString> descriptions = {
        {"RUNSPEC", "simulation run specifications and dimensions"},
        {"GRID", "grid geometry and properties"},
        {"EDIT", "grid modifications and editing operations"},
        {"PROPS", "fluid and rock properties"},
        {"REGIONS", "region definitions and properties"},
        {"SOLUTION", "initial conditions and equilibration"},
        {"SUMMARY", "output requests and reporting"},
        {"SCHEDULE", "well operations and field development"}
    };
    
    return descriptions.value(section.toUpper(), "simulation parameters");
}