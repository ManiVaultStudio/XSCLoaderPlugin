#include <QStyle>
#include "QtTooltipKit.h"
#include "InputDialogCSV.h"
#include "PointData/PointData.h"

InputDialogCSV::InputDialogCSV(QWidget* parent, std::string& filePath, std::string checkTypeValue, QStringList headers) :
    QDialog(parent)
{
    setWindowTitle(tr("Cross Species Comparison Data Loader"));
    std::string fullfileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    _dataNameValue = new QLineEdit();
    _dataNameValue->setText(QString::fromStdString(fullfileName.substr(0, (fullfileName.find_last_of(".")))));

    messageValue = new QLabel();  // Use QLabel instead of QLineEdit
    messageValue->setWordWrap(false);  // Allow text to wrap within QLabel
    messageValue->setAlignment(Qt::AlignTop | Qt::AlignCenter); // Adjust alignment as needed
    messageValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    if (checkTypeValue == "AllData")
    {
        messageValue->setText("Please choose the option below:");
        messageValue->setStyleSheet("QLabel { color : black; font-size: 12pt; }");  // Adjust styles as needed
    }
    else if (checkTypeValue == "MetaData")
    {
        messageValue->setText("Data can can be loaded as metadata!");
        messageValue->setStyleSheet("QLabel { color : black;  font-size: 12pt; }");  // Adjust styles as needed
    }
    else if (checkTypeValue == "NormalData")
    {
        messageValue->setText("Data can can be loaded as point and cluster data!");
        messageValue->setStyleSheet("QLabel { color : black;  font-size: 12pt; }");  // Adjust styles as needed
    }
    else
    {
        messageValue->setText("Data cannot be loaded! Please check the data format and try again.");
        messageValue->setStyleSheet("QLabel { color : red;  font-size: 12pt; }");  // Adjust styles as needed
    }
    addButton = new QPushButton(tr("add file"));
    addButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    qttooltipkit::set(*addButton, tr("Import dataset"), tr("Load the selected file using the options in this dialog."));
    addButton->setDefault(true);


    allDataType = new QRadioButton("Point and Cluster");
    metaDataType = new QRadioButton("Meta");
    metaDataType->setChecked(true);
    //allDataType->setChecked(true);

    okButton = new QPushButton(tr("close"));
    okButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    qttooltipkit::set(*okButton, tr("Close importer"), tr("Dismiss this dialog."));
    okButton->setDefault(true);
    _leafOptionValues = new QComboBox();
    _leafOptionValues->addItems(headers);
    qttooltipkit::set(*_leafOptionValues, tr("Species identifier column"), tr("Choose the CSV column whose values match the leaf names in your reference tree."));

    _leafOptionValues->setCurrentIndex(0);

    const auto allDataTypeVals = [this]() -> void
        {
            if (allDataType->isChecked())
            {
                _leafOptionValues->setEnabled(false);
            }

        };
    const auto metaDataTypeVals = [this]() -> void
        {
            if (metaDataType->isChecked())
            {
                _leafOptionValues->setEnabled(true);
            }

        };
    connect(allDataType, &QRadioButton::clicked, this, allDataTypeVals);
    connect(metaDataType, &QRadioButton::clicked, this, metaDataTypeVals);
    
    connect(okButton, &QPushButton::pressed, this, &InputDialogCSV::okDialogAction);

    connect(addButton, &QPushButton::pressed, this, &InputDialogCSV::closeDialogAction);
    connect(this, &InputDialogCSV::closeDialogCSV, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout();

    layout->addWidget(messageValue);

    if (checkTypeValue == "MetaData" || checkTypeValue == "AllData" || checkTypeValue == "NormalData")
    {


        auto labelDataNameValue = std::make_unique<QLabel>(tr("&File name: "));
        labelDataNameValue->setBuddy(_dataNameValue);
        layout->addWidget(labelDataNameValue.release());
        layout->addWidget(_dataNameValue);
        if (checkTypeValue == "AllData" )
        {

            QHBoxLayout* dataTypeLayout = new QHBoxLayout();
            dataTypeLayout->addWidget(metaDataType);
            dataTypeLayout->addWidget(allDataType);



            auto labelDataTypeValue = new QLabel(tr("Data type: "));
            layout->addWidget(labelDataTypeValue);
            layout->addLayout(dataTypeLayout);

        }
        else if (checkTypeValue == "MetaData")
        {
            metaDataType->setChecked(true);
            allDataType->setChecked(false);
        }
        if (checkTypeValue == "NormalData")
        {
            metaDataType->setChecked(false);
            allDataType->setChecked(true);
        }
        else
        {
            auto leafnamesTypeValue = new QLabel(tr("Leaf names: "));
            layout->addWidget(leafnamesTypeValue);
            layout->addWidget(_leafOptionValues);

        }

            
        layout->addWidget(addButton);
    }

    else
    {
        layout->addWidget(okButton);
    }




    setLayout(layout);
    adjustSize();
}

void InputDialogCSV::closeDialogAction()
{
    if(allDataType->isChecked())
    {
               emit closeDialogCSV(_dataNameValue->text(), "Normal", _leafOptionValues->currentText());
    }
    else if(metaDataType->isChecked())
    {
               emit closeDialogCSV(_dataNameValue->text(), "Meta", _leafOptionValues->currentText());
    }

    //emit closeDialog(_dataNameValue->text());
}


void InputDialogCSV::okDialogAction()
{
    accept();
}