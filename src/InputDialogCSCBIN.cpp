#include <QStyle>
#include "QtTooltipKit.h"
#include "InputDialogCSCBIN.h"
#include "PointData/PointData.h"

InputDialogCSCBIN::InputDialogCSCBIN(QWidget* parent, std::string& filePath,QString checkTypeValue) :
    QDialog(parent)
{
    setWindowTitle(tr("Cross Species Comparison CSCBIN Data Loader"));   
    std::string fullfileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    _dataNameValue = new QLineEdit();
    _dataNameValue->setText(QString::fromStdString(fullfileName.substr(0, (fullfileName.find_last_of(".")))));

    messageValue = new QLabel();  // Use QLabel instead of QLineEdit
    messageValue->setWordWrap(false);  // Allow text to wrap within QLabel
    messageValue->setAlignment(Qt::AlignTop | Qt::AlignCenter); // Adjust alignment as needed
    messageValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    if (checkTypeValue == "Processed")
    {
        messageValue->setText("Data can be loaded! Please choose the option below to proceed.");
        messageValue->setStyleSheet("QLabel { color : black; font-size: 12pt; }");  // Adjust styles as needed
    }
    else
    {
        messageValue->setText("Data cannot be loaded! "+ checkTypeValue +" Please check the data format and try again.");
        messageValue->setStyleSheet("QLabel { color : red;  font-size: 12pt; }");  // Adjust styles as needed
    }
    addButton = new QPushButton(tr("add file"));
    addButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    qttooltipkit::set(*addButton, tr("Import dataset"), tr("Load the selected file using the options in this dialog."));
    addButton->setDefault(true);

    okButton = new QPushButton(tr("close"));
    okButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    qttooltipkit::set(*okButton, tr("Close importer"), tr("Dismiss this dialog."));
    okButton->setDefault(true);

    connect(okButton, &QPushButton::pressed, this, &InputDialogCSCBIN::okDialogAction);

    connect(addButton, &QPushButton::pressed, this, &InputDialogCSCBIN::closeDialogAction);
    connect(this, &InputDialogCSCBIN::closeDialogCSCBIN, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout();

    layout->addWidget(messageValue);
    if(checkTypeValue == "Processed")
    {
        auto labelDataNameValue = std::make_unique<QLabel>(tr("&File name: "));
        labelDataNameValue->setBuddy(_dataNameValue);
        layout->addWidget(labelDataNameValue.release());
        layout->addWidget(_dataNameValue);

        layout->addWidget(addButton);
    }
    else
    {
        layout->addWidget(okButton);
    }

    setLayout(layout);
    adjustSize();
}

void InputDialogCSCBIN::closeDialogAction()
{

    emit closeDialogCSCBIN(_dataNameValue->text(),"Done");
}


void InputDialogCSCBIN::okDialogAction()
{
    accept();
}