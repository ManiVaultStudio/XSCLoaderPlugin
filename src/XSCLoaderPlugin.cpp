#include "XSCLoaderPlugin.h"

#include "PointData/PointData.h"
#include "ClusterData/ClusterData.h"
#include "Set.h"
#include <vector>
#include <QtCore>
#include <QtDebug>
#include <random>
#include <QInputDialog>
#include <QSet>
#include <QString>
#include <QTextStream>
#include <QStringList>
#include <algorithm>
#include <QSet>
#include <set>
#include <iostream>
#include <string>
#include <map>
#include <limits.h>
#include <sstream>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <sstream>


Q_PLUGIN_METADATA(IID "nl.tudelft.XSCLoaderPlugin")

using namespace mv;

// =============================================================================
// View
// =============================================================================

XSCLoaderPlugin::XSCLoaderPlugin(const PluginFactory* factory):
    LoaderPlugin(factory),
    _dataSetName("")
{
    //_infoSettingsAction.setDefaultWidgetFlags(StringAction::TextEdit);
    //_infoSettingsAction.setShowLabels(false);
    //_infoSettingsAction.setDisabled(true);
}

XSCLoaderPlugin::~XSCLoaderPlugin(void)
{
    
}

/**
 * Mandatory plugin override function. Any initial state can be set here.
 * This function gets called when an instance of the plugin is created.
 * In this case when someone select the loader option from the menu.
 */
void XSCLoaderPlugin::init()
{

}

/**
 * Mandatory override function. Gets called when someone selects the XSC Loader option
 * from the menu containing loader plugins. This function is responsible for opening the
 * file(s) the user wants to open, process them appropriately and pass the final data to the core.
 */

bool areAllStrings(const QStringList& list) {
    for (const QString& str : list) {
        if (str.toInt() != 0 || str.isEmpty()) {
            // If conversion to int is successful or the string is empty,
            // it's not a pure string value
            return false;
        }
    }
    return true;
}



bool areAllNumeric(const QStringList& list) {
    for (const QString& str : list) {
        if (str.isEmpty() || str.toInt() == 0) {
            // If conversion to int is unsuccessful or the result is zero, it's not a numeric value
            return false;
        }
    }
    return true;
}

bool areAllValuesUnique(const QStringList& list) {
    QSet<QString> uniqueSet;

    for (const QString& str : list) {
        if (uniqueSet.contains(str)) {
            return false;  // Found a duplicate
        }
        uniqueSet.insert(str);
    }

    return true;  // All values are unique
}

bool areAllValuesNonEmpty(const QStringList& list) {
    // Check if all values are non-empty
    for (const QString& str : list) {
        if (str.isEmpty()) {
            return false;  // Found an empty string
        }
    }
    return true;  // All values are non-empty
}

bool areListsEqual(const QStringList& list1, const QStringList& list2) {
    if (list1.size() != list2.size()) {
        return false;  // Different sizes, can't be equal
    }

    for (int i = 0; i < list1.size(); ++i) {
        if (list1.at(i) != list2.at(i)) {
            return false;  // Different elements at the same position
        }
    }

    return true;  // Lists are equal
}

bool areAllValuesNumeric(const std::vector<QStringList>& data) {
    for (const auto& strList : data) {
        for (const QString& str : strList) {
            bool isNumeric;
            str.toDouble(&isNumeric);  // Try to convert to a double
            if (!isNumeric) {
                return false;  // Found a non-numeric value
            }
        }
    }
    return true;  // All values are numeric
}
bool areAllValuesZeroOrOne(const std::vector<QStringList>& data) {
    for (const auto& strList : data) {
        for (const QString& str : strList) {
            bool isNumeric;
            double numericValue = str.toDouble(&isNumeric);

            if (!isNumeric || (numericValue != 0.0 && numericValue != 1.0)) {
                return false;  // Found a non-0 or non-1 value
            }
        }
    }
    return true;  // All values are 0 or 1
}


bool areAllValuesNonEmpty(const std::vector<QStringList>& loadedData) {
    for (const auto& tokens : loadedData) {
        for (const QString& value : tokens) {
            if (value.isEmpty()) {
                return false; // At least one value is empty
            }
        }
    }
    return true; // All values are non-empty
}
bool isNumeric(const QString& str)
{
    bool ok;
    QString copy = str;
    copy.remove(',');
    copy.toDouble(&ok);
    return ok;
}
bool hasNumericColumn(const std::vector<QStringList>& loadedData)
{
    for (const auto& row : loadedData)
    {
        for (const QString& element : row)
        {
            if (isNumeric(element))
            {
                return true; // At least one column is numeric
            }
        }
    }
    return false; // No numeric column found
}


QJsonObject convertJsonArray(QJsonObject& jsonObject, int& id) {
    QJsonObject newObject;

    if (jsonObject.contains("name")) {
        newObject["name"] = jsonObject["name"].toString();
        newObject["branchLength"] = jsonObject.contains("branchLength") ? jsonObject["branchLength"].toDouble() : 1.0;
        newObject["cellCounts"] = jsonObject.contains("cellCounts") ? jsonObject["cellCounts"].toDouble() : 0.0;
        newObject["mean"] = jsonObject.contains("mean") ? jsonObject["mean"].toDouble() : 0.0;
        newObject["differential"] = jsonObject.contains("differential") ? jsonObject["differential"].toDouble() : 0.0;
        newObject["abundanceTop"] = jsonObject.contains("abundanceTop") ? jsonObject["abundanceTop"].toDouble() : 0.0;
        newObject["abundanceMiddle"] = jsonObject.contains("abundanceMiddle") ? jsonObject["abundanceMiddle"].toDouble() : 0.0;
        newObject["rank"] = jsonObject.contains("rank") ? jsonObject["rank"].toInt() : 0;
        newObject["gene"] = jsonObject.contains("gene") ? jsonObject["gene"].toString() : " ";
        newObject["clusterName"] = jsonObject.contains("clusterName") ? jsonObject["clusterName"].toString() : " ";
        newObject["middleAbundanceClusterName"] = jsonObject.contains("middleAbundanceClusterName") ? jsonObject["middleAbundanceClusterName"].toString() : " ";
        newObject["color"] = jsonObject.contains("color") ? jsonObject["color"].toString() : "#000000";
        newObject["hastrait"] = jsonObject.contains("hastrait") ? jsonObject["hastrait"].toBool() : true;
        newObject["iscollapsed"] = jsonObject.contains("iscollapsed") ? jsonObject["iscollapsed"].toBool() : true;
    }

    if (jsonObject.contains("children")) {
        QJsonArray childrenArray = jsonObject["children"].toArray();
        QJsonArray newChildrenArray;

        for (int i = 0; i < childrenArray.size(); i++) {
            QJsonObject childObject = childrenArray[i].toObject();
            newChildrenArray.append(convertJsonArray(childObject, id));
        }

        newObject["children"] = newChildrenArray;

        // If the object has a "name" field, it's a leaf node and should not have "id", "score", and "width" fields
        if (!jsonObject.contains("name")) {
            newObject["id"] = id++;
            newObject["branchLength"] = jsonObject.contains("branchLength") ? jsonObject["branchLength"].toDouble() : 1.0;
            newObject["score"] = jsonObject.contains("score") ? jsonObject["score"].toDouble() : 1.0;
            newObject["width"] = jsonObject.contains("width") ? jsonObject["width"].toDouble() : 1.0;
        }
    }

    return newObject;
}

/*
void XSCLoaderPlugin::saveBinSet(const BinSet& binSet, const std::string& filename) {
    std::ostringstream oss;
    // Write secret signature
    std::string secretSignature = "XSCSaverPlugin_Signature_Verify";
    oss << secretSignature << "\n";
    // Serialize and write DataMain
    oss << binSet.dataMain.rows << "\n";
    oss << binSet.dataMain.columns << "\n";
    for (const auto& value : binSet.dataMain.values) {
        oss << value << "\n";
    }
    for (const auto& name : binSet.dataMain.dimensionNames) {
        oss << name << "\n";
    }
    oss << binSet.dataMain.mainDatasetName << "\n"; // Added line to serialize mainDatasetName

    // Serialize and write DataClusterForADataset
    oss << binSet.dataClustersDerived.size() << "\n";
    for (const auto& dataCluster : binSet.dataClustersDerived) {
        oss << dataCluster.clusterValues.size() << "\n";
        for (const auto& singleCluster : dataCluster.clusterValues) {
            oss << singleCluster.clusterName << "\n";
            oss << singleCluster.clusterColor << "\n";
            oss << singleCluster.clusterIndices.size() << "\n";
            for (const auto& index : singleCluster.clusterIndices) {
                oss << index << "\n";
            }
        }
        oss << dataCluster.derivedclusterdatasetName << "\n"; // Added line to serialize derivedclusterdatasetName
    }

    // Serialize and write DataPointsDerived
    oss << binSet.dataPointsDerived.size() << "\n";
    for (const auto& dataPoint : binSet.dataPointsDerived) {
        oss << dataPoint.rows << "\n";
        oss << dataPoint.columns << "\n";
        for (const auto& value : dataPoint.values) {
            oss << value << "\n";
        }
        for (const auto& name : dataPoint.dimensionNames) {
            oss << name << "\n";
        }
        oss << dataPoint.derivedpointdatasetName << "\n"; // Added line to serialize derivedpointdatasetName
    }

    std::string uncompressedData = oss.str();

    uLongf compressedDataSize = compressBound(uncompressedData.size());
    std::vector<char> compressedData(compressedDataSize);
    if (compress((Bytef*)compressedData.data(), &compressedDataSize, (const Bytef*)uncompressedData.data(), uncompressedData.size()) != Z_OK) {
        std::cerr << "Failed to compress data" << std::endl;
        return;
    }

    compressedData.resize(compressedDataSize); // Resize to actual compressed size

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "File could not be opened for writing: " << filename << std::endl;
        return;
    }

    outFile.write(compressedData.data(), compressedData.size());
    outFile.close();
}



std::pair<BinSet, QString> XSCLoaderPlugin::readBinSet(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "File could not be opened for reading: " << filename << std::endl;
        return { BinSet{}, QString("File could not be opened for reading: %1").arg(QString::fromStdString(filename)) };
    }

    std::string compressedData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());

    uLongf decompressedDataSize = compressedData.size() * 10; // Estimate decompressed size
    std::vector<char> decompressedData(decompressedDataSize);
    if (uncompress((Bytef*)decompressedData.data(), &decompressedDataSize, (const Bytef*)compressedData.data(), compressedData.size()) != Z_OK) {
        std::cerr << "Failed to decompress data" << std::endl;
        return { BinSet{}, QString("Failed to decompress data: %1").arg(QString::fromStdString(filename)) };
    }

    decompressedData.resize(decompressedDataSize); // Resize to actual decompressed size

    std::istringstream iss(std::string(decompressedData.begin(), decompressedData.end()));

    // Read and check secret signature
    std::string secretSignature;
    std::getline(iss, secretSignature);
    if (secretSignature != "XSCSaverPlugin_Signature_Verify") {
        std::cerr << "Invalid file signature: " << filename << std::endl;
        return { BinSet{}, QString("Invalid file signature: %1").arg(QString::fromStdString(filename)) };
    }

    BinSet binSet;

    // Deserialize and read DataMain
    iss >> binSet.dataMain.rows;
    iss >> binSet.dataMain.columns;
    binSet.dataMain.values.resize(binSet.dataMain.rows * binSet.dataMain.columns);
    for (auto& value : binSet.dataMain.values) {
        iss >> value;
    }
    iss.ignore(); // To skip the newline character
    binSet.dataMain.dimensionNames.resize(binSet.dataMain.columns);
    for (auto& name : binSet.dataMain.dimensionNames) {
        std::getline(iss, name);
    }
    std::getline(iss, binSet.dataMain.mainDatasetName); // Added line to deserialize mainDatasetName

    // Deserialize and read DataClusterForADataset
    size_t numDataClusters;
    iss >> numDataClusters;
    iss.ignore(); // To skip the newline character
    binSet.dataClustersDerived.resize(numDataClusters);
    for (auto& dataCluster : binSet.dataClustersDerived) {
        size_t numClusterValues;
        iss >> numClusterValues;
        iss.ignore(); // To skip the newline character
        dataCluster.clusterValues.resize(numClusterValues);
        for (auto& singleCluster : dataCluster.clusterValues) {
            std::getline(iss, singleCluster.clusterName);
            std::getline(iss, singleCluster.clusterColor);
            size_t numIndices;
            iss >> numIndices;
            iss.ignore(); // To skip the newline character
            singleCluster.clusterIndices.resize(numIndices);
            for (auto& index : singleCluster.clusterIndices) {
                iss >> index;
            }
            iss.ignore(); // To skip the newline character
        }
        std::getline(iss, dataCluster.derivedclusterdatasetName); // Added line to deserialize derivedclusterdatasetName
    }

    // Deserialize and read DataPointsDerived
    size_t numDataPoints;
    iss >> numDataPoints;
    iss.ignore(); // To skip the newline character
    binSet.dataPointsDerived.resize(numDataPoints);
    for (auto& dataPoint : binSet.dataPointsDerived) {
        iss >> dataPoint.rows;
        iss >> dataPoint.columns;
        dataPoint.values.resize(dataPoint.rows * dataPoint.columns);
        for (auto& value : dataPoint.values) {
            iss >> value;
        }
        iss.ignore(); // To skip the newline character
        dataPoint.dimensionNames.resize(dataPoint.columns);
        for (auto& name : dataPoint.dimensionNames) {
            std::getline(iss, name);
        }
        std::getline(iss, dataPoint.derivedpointdatasetName); // Added line to deserialize derivedpointdatasetName
    }

    inFile.close();

    return { binSet, QString("Processed") };
}

*/


void XSCLoaderPlugin::loadData()
{

    //const auto fileName = AskForFileName(QObject::tr("JSON and CSV Files (*.json *.csv)"));
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        "Open File",
        "",
        "JSON and CSV Files (*.json *.csv)"
    );
    checkTypeValue = "None";
    if (fileName.isEmpty())
    {
        return;
    }
    if (fileName.endsWith(".csv") || fileName.endsWith(".json"))
    {
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            throw DataLoadException(fileName, "File was not found at location.");

        if (fileName.endsWith(".csv")) {
            qDebug() << "Loading CSV file: " << fileName;


            QTextStream in(&file);

            if (!file.atEnd()) {
                // Read data from the file
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QRegularExpression regex("\"([0-9]+),([0-9]+)\"");
                    line.replace(regex, "\\1\\2");
                    line.replace("\"", "");
                    QStringList tokens = line.split(",");
                    _loadedData.push_back(tokens);
                }
            }
            else
            {
                throw DataLoadException(fileName, "File is empty.");
            }
            // Remove last empty rows
            while (!_loadedData.empty() && _loadedData.back().isEmpty())
            {
                _loadedData.pop_back();
            }

            // Remove last empty columns
            if (!_loadedData.empty())
            {
                int maxColumns = _loadedData[0].size();

                for (int i = _loadedData.size() - 1; i >= 0; --i)
                {
                    QStringList& row = _loadedData[i];

                    while (!row.isEmpty() && row.back().isEmpty())
                    {
                        row.pop_back();
                    }

                    if (row.size() < maxColumns)
                    {
                        // Add empty strings to make the row size equal to maxColumns
                        row += QStringList(maxColumns - row.size(), "");
                    }
                }
            }
            //  Extract header row 
            QStringList headerRow = _loadedData[0];
            std::map<QString, int> headerCount;
            for (int i = 0; i < headerRow.size(); ++i) {
                QString& header = headerRow[i];
                if (headerCount.count(header) > 0) {
                    // If the header is already in the map, increment its count and append the count to the header
                    header = header + "_" + QString::number(++headerCount[header]);
                }
                else {
                    // If the header is not in the map, add it with a count of 1
                    headerCount[header] = 1;
                }
            }

            // Update the header row in the loaded data
            _loadedData[0] = headerRow;
            //headerRow.removeLast(); //without the last column name
            QStringList uniqueStringColumns;
            if (!_loadedData.empty())
            {


                // Create a list to store the names of columns that only contain unique string values


                // Iterate over each column
                for (int col = 0; col < headerRow.size(); ++col) {
                    QStringList columnData;

                    // Gather the data for this column
                    for (int row = 1; row < _loadedData.size(); ++row) {  // Start from 1 to skip the header row
                        columnData.push_back(_loadedData[row][col]);
                    }

                    // Check if all values in the column are strings and unique
                    if (areAllStrings(columnData) && areAllValuesUnique(columnData)) {
                        uniqueStringColumns.push_back(headerRow[col]);
                    }
                }
                bool checkNumeric = hasNumericColumn(_loadedData);

                if (uniqueStringColumns.count() > 0 && checkNumeric)
                {
                    checkTypeValue = "AllData";
                }

                else if (uniqueStringColumns.count() > 0)
                {
                    checkTypeValue = "MetaData";
                }

                else if (hasNumericColumn(_loadedData))
                {
                    checkTypeValue = "NormalData";
                }


            }


            // Gather some knowledge about the data from the user
            auto fileNameString = fileName.toStdString();
            InputDialogCSV inputDialog(nullptr, fileNameString, checkTypeValue, uniqueStringColumns);
            inputDialog.setModal(true);

            connect(&inputDialog, &InputDialogCSV::closeDialogCSV, this, &XSCLoaderPlugin::dialogClosedCSV);


            int inputOk = inputDialog.exec();


        }
        else if (fileName.endsWith(".json")) {
            qDebug() << "Loading JSON file: " << fileName;
            /*
             QString jsonInput = R"([
       {"value1": "Human", "value2": "Chimpanzee", "cluster": "Cluster 1"},
       {"value1": "Gorilla", "value2": "Cluster 1", "cluster": "Cluster 2"},
       {"value1": "Rhesus", "value2": "Cluster 2", "cluster": "Cluster 3"},
       {"value1": "Marmoset", "value2": "Cluster 3", "cluster": "Cluster 4"}
       ])";
                   */
            QString message = "";
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                throw DataLoadException(fileName, "File was not found at location.");

            QByteArray jsonData = file.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(jsonData);

            if (!doc.isNull()) {
                if (doc.isObject()) {
                    QJsonObject jsonObject = doc.object();

                    int id = 1;
                    _treeData = convertJsonArray(jsonObject, id);

                    //std::cout << QJsonDocument(_treeData).toJson().toStdString() << std::endl;

                    message = "Processed";
                }
                else {
                    qDebug() << "Document is not an array";
                    message = "Document is not an array";
                }
            }
            else {
                qDebug() << "Invalid JSON...\n";// << jsonData;
                message = "Invalid JSON...";
            }

            // Gather some knowledge about the data from the user
            auto fileNameString = fileName.toStdString();
            //checkTypeValue = "None"; //"XSCTree" or "Trait" or "None"
            InputDialogJSON inputDialog(nullptr, fileNameString, message);
            inputDialog.setModal(true);

            connect(&inputDialog, &InputDialogJSON::closeDialogJSON, this, &XSCLoaderPlugin::dialogClosedJSON);


            int inputOk = inputDialog.exec();
        }
    }
    /*
    else if (fileName.endsWith(".cscbin"))
        { 
            
    auto fileNameString = fileName.toStdString();
    QString message;
    std::tie(_binSetRead, message) = readBinSet(fileNameString);
  
    InputDialogCSCBIN inputDialog(nullptr, fileNameString, message);
    inputDialog.setModal(true);

    connect(&inputDialog, &InputDialogCSCBIN::closeDialogCSCBIN, this, &XSCLoaderPlugin::dialogClosedCSCBIN);

    int inputOk = inputDialog.exec();
        }
        */
}

// Function to generate a distance matrix
std::vector<std::vector<float>> generateDistanceMatrix(const std::vector<ClusterInput>& clusterOrder, std::vector<QString> species) {
    // Sort species in alphabetical order
    //std::sort(species.begin(), species.end());

    int numSpecies = species.size();

    // Initialize distance matrix with 0
    std::vector<std::vector<float>> distanceMatrix(numSpecies, std::vector<float>(numSpecies, 0));

    // Create a map to store the desired order of clustering
    std::map<std::string, int> orderMap;
    int order = 0;
    for (const auto& input : clusterOrder) {
        if (orderMap.count(input.value1) == 0) {
            orderMap[input.value1] = order++;
        }
        if (orderMap.count(input.value2) == 0) {
            orderMap[input.value2] = order++;
        }
    }

    // Set distances based on desired clustering
    for (int i = 0; i < numSpecies; ++i) {
        for (int j = i + 1; j < numSpecies; ++j) {
            // Set distance based on the order in which clusters should be merged
            int order_i = (orderMap.count(species[i].toStdString()) > 0) ? orderMap[species[i].toStdString()] : INT_MAX;
            int order_j = (orderMap.count(species[j].toStdString()) > 0) ? orderMap[species[j].toStdString()] : INT_MAX;
            distanceMatrix[i][j] = distanceMatrix[j][i] = std::max(order_i, order_j);
        }
    }

    return distanceMatrix;
}


void storeColumns(const std::vector<QStringList>& loadedData, std::map<int, std::vector<float>>& numericColumns, std::map<int, std::vector<std::string>>& stringColumns, std::map<int, std::string>& columnNames) {
    // Assuming headerRow is already initialized
    QStringList headerRow = loadedData[0];
    int numRows = loadedData.size() - 1; // Number of rows excluding the header

    for (int col = 0; col < headerRow.size(); ++col) {
        // Get column name
        std::string columnName = headerRow[col].toStdString();
        columnNames[col] = columnName;

        // Check if the column is numeric
        bool isNumericColumn = true;
        for (int row = 1; row < loadedData.size(); ++row) {
            bool isNumericValue;
            loadedData[row][col].toFloat(&isNumericValue);
            if (!isNumericValue) {
                isNumericColumn = false;
                break;
            }
        }

        // Store values based on whether the column is numeric or not
        if (isNumericColumn) {
            std::vector<float> numericValues;
            for (int row = 1; row <= numRows; ++row) {
                float value = -1.0; // Placeholder for missing numeric value
                if (row < loadedData.size()) {
                    bool isNumericValue;
                    value = loadedData[row][col].toFloat(&isNumericValue);
                    if (!isNumericValue) {
                        value = -1.0; // If the value is not numeric, use the placeholder
                    }
                }
                numericValues.push_back(value);
            }
            numericColumns[col] = numericValues;
        }
        else {
            std::vector<std::string> stringValues;
            for (int row = 1; row <= numRows; ++row) {
                std::string value = "None"; // Placeholder for missing non-numeric value
                if (row < loadedData.size()) {
                    value = loadedData[row][col].toStdString();
                }
                stringValues.push_back(value);
            }
            stringColumns[col] = stringValues;
        }
    }
}

void XSCLoaderPlugin::dialogClosedJSON(QString dataSetName, QString typeName)
{
    // Generate distance matrix
    std::vector<std::vector<float>> distanceMatrix = generateDistanceMatrix(_speciesOrder, _speciesNames);

    Dataset<XSCTree> treeDataset = mv::data().createDataset("XSCTree", dataSetName + "_Tree");
    events().notifyDatasetAdded(treeDataset);
    treeDataset->setTreeData(_treeData);
    QJsonDocument jsonDoc(_treeData);
    QString jsonString = jsonDoc.toJson();
    //auto data= treeDataset->getFullDataset<XSCTree>()->getData();

    /*auto datasetName = dataSetName + "_Tree";
    Dataset<Points> valuesDataset = mv::data().createDataset("Points", datasetName);
    

    std::vector<float> flatMatrix;
    int numRows = distanceMatrix.size();
    int numCols = numRows > 0 ? distanceMatrix[0].size() : 0;

    for (const auto& row : distanceMatrix) {
        for (const auto& element : row) {
            flatMatrix.push_back(element);
        }
    }

    valuesDataset->setData(flatMatrix.data(), numRows, numCols);
    valuesDataset->setDimensionNames(_speciesNames);*/

    events().notifyDatasetDataChanged(treeDataset);

    /*
    // Create an output string stream
    std::ostringstream oss;

    // Iterate over the vector and concatenate elements with commas
    for (size_t i = 0; i < _speciesNames.size(); ++i) {
        if (i != 0) {
            oss << ",";
        }
        oss << _speciesNames[i];
    }

    // Get the resulting string
    std::string result = oss.str();

    // Print the resulting string
    std::cout << result << std::endl;

    int speciesCounter = 0;
    // Print distance matrix
    for (const auto& row : distanceMatrix) {
        for (int dist : row) {
            std::cout << dist << ',';
        }
        std::cout << _speciesNames[speciesCounter];
        speciesCounter = speciesCounter + 1;
        std::cout << '\n';
    } */
    
}

void XSCLoaderPlugin::dialogClosedCSCBIN(QString dataSetName, QString TypeName)
{

 

       /* qDebug() << "After reading:";
        qDebug() << "DataMain rows: " << _binSetRead.dataMain.rows;
        qDebug() << "DataMain columns: " << _binSetRead.dataMain.columns;
        qDebug() << "First value in DataMain values: " << _binSetRead.dataMain.values[0];
        qDebug() << "First dimension name in DataMain: " << QString::fromStdString(_binSetRead.dataMain.dimensionNames[0]);
        */

        DataMain mainData= _binSetRead.dataMain;
        std::vector<DataClusterForADataset> clusterDataContainer = _binSetRead.dataClustersDerived;
        std::vector<DataPointsDerived> derivedPointsDataContainer= _binSetRead.dataPointsDerived;


        //processMainData
        int NumberOfRows= mainData.rows;
        int mainDataNumOfDims = mainData.columns;
        std::vector<QString> mainDataDimensionNames;
        for (const auto& str : mainData.dimensionNames) {
            mainDataDimensionNames.push_back(QString::fromStdString(str));
        }

        Dataset<Points> pointValuesDataset = mv::data().createDataset("Points", dataSetName);
        events().notifyDatasetAdded(pointValuesDataset);
        pointValuesDataset->setData(mainData.values.data(), NumberOfRows, mainData.columns);
        pointValuesDataset->setDimensionNames(mainDataDimensionNames);
        events().notifyDatasetDataChanged(pointValuesDataset);

        //process Derived PointData
        for (auto derivedPointsData : derivedPointsDataContainer)
        {
            
            QString datasetName = "Blank";
                if (derivedPointsData.derivedpointdatasetName!="")
                {
                    datasetName = QString::fromStdString(derivedPointsData.derivedpointdatasetName);
                }
            Dataset<Points>  derivedPointsDataset = mv::data().createDataset("Points", datasetName, pointValuesDataset);
            events().notifyDatasetAdded(derivedPointsDataset);
            derivedPointsDataset->setData(derivedPointsData.values.data(), derivedPointsData.rows, derivedPointsData.columns);

            std::vector<QString> derivedDataDimensionNames;
            for (const auto& str : derivedPointsData.dimensionNames) {
                derivedDataDimensionNames.push_back(QString::fromStdString(str));
            }
            derivedPointsDataset->setDimensionNames(derivedDataDimensionNames);
            events().notifyDatasetDataChanged(derivedPointsDataset);
        }

        //processDerivedClusterData
        for (auto clusterDatasets : clusterDataContainer)
        {
            Dataset<Clusters>  clusterValuesDataset = mv::data().createDataset("Cluster", QString::fromStdString(clusterDatasets.derivedclusterdatasetName), pointValuesDataset);
            events().notifyDatasetAdded(clusterValuesDataset);
            
            for (auto clusterDetails : clusterDatasets.clusterValues)
            {
                Cluster cluster;
                cluster.setName(QString::fromStdString(clusterDetails.clusterName));
                std::vector<uint32_t> indices(clusterDetails.clusterIndices.begin(), clusterDetails.clusterIndices.end());
                cluster.setIndices(indices);
                cluster.setColor(QColor(QString::fromStdString(clusterDetails.clusterColor)));
                clusterValuesDataset->addCluster(cluster);
            }
            events().notifyDatasetDataChanged(clusterValuesDataset);

        }

}

std::vector<QString> XSCLoaderPlugin::extractStringColumnValues(int columnIndex) {
    std::vector<QString> columnValues;

    // Check if the column index is valid
    if (columnIndex < 0 || columnIndex >= _loadedData[0].size()) {
        qWarning() << "Invalid column index: " << columnIndex;
        return columnValues;
    }

    // Gather the data for this column
    for (int row = 1; row < _loadedData.size(); ++row) {  // Start from 1 to skip the header row
        columnValues.push_back(_loadedData[row][columnIndex]);
    }

    return columnValues;
}

std::vector<float> XSCLoaderPlugin::extractNumericColumnValues(int columnIndex) {
    std::vector<float> columnValues;

    // Check if the column index is valid
    if (columnIndex < 0 || columnIndex >= _loadedData[0].size()) {
        qWarning() << "Invalid column index: " << columnIndex;
        return columnValues;
    }

    // Gather the data for this column
    for (int row = 1; row < _loadedData.size(); ++row) {  // Start from 1 to skip the header row
        bool isNumeric;
        float value = _loadedData[row][columnIndex].toFloat(&isNumeric);
        if (isNumeric) {
            columnValues.push_back(value);
        }
        else {
            qWarning() << "Non-numeric value in numeric column: " << _loadedData[row][columnIndex];
        }
    }

    return columnValues;
}

std::pair<std::vector<int>, std::vector<int>> XSCLoaderPlugin::getColumnIndexes() {
    std::vector<int> stringColumnIndexes;
    std::vector<int> numericColumnIndexes;

    // Get the column names
    QStringList columnNames = _loadedData[0];

    // Iterate over each column
    for (int col = 0; col < columnNames.size(); ++col) {
        QStringList columnData;

        // Gather the data for this column
        for (int row = 1; row < _loadedData.size(); ++row) {  // Start from 1 to skip the header row
            columnData.push_back(_loadedData[row][col]);
        }

        // Check if any values in the column are strings
        if (std::any_of(columnData.begin(), columnData.end(), [](const QString& str) { return !isNumeric(str); })) {
            stringColumnIndexes.push_back(col);
        }

        // Check if any values in the column are numeric
        else if (std::any_of(columnData.begin(), columnData.end(), [](const QString& str) { return isNumeric(str); })) {
            numericColumnIndexes.push_back(col);
        }
    }

    return std::make_pair(stringColumnIndexes, numericColumnIndexes);
}


std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> XSCLoaderPlugin::getValueIndexes() {
    std::vector<int> stringColumnIndexes;
    std::vector<int> numericColumnIndexes;
    std::vector<int> colorColumnIndexes;

    // Get the column names
    QStringList columnNames = _loadedData[0];

    // Regular expression for hexadecimal color code
    QRegularExpression colorRegex("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})$");

    // Iterate over each column
    for (int col = 0; col < columnNames.size(); ++col) {
        QStringList columnData;

        // Gather the data for this column
        for (int row = 1; row < _loadedData.size(); ++row) {  // Start from 1 to skip the header row
            columnData.push_back(_loadedData[row][col]);
        }

        // Check if any values in the column are strings
        if (std::any_of(columnData.begin(), columnData.end(), [](const QString& str) { return !isNumeric(str); })) {
            // Check if any values in the column are colors
            if (std::any_of(columnData.begin(), columnData.end(), [&colorRegex](const QString& str) { return colorRegex.match(str).hasMatch(); })) {
                colorColumnIndexes.push_back(col);
            }
            else {
                stringColumnIndexes.push_back(col);
            }
        }
        else {  // If all values are numeric
            numericColumnIndexes.push_back(col);
        }
    }

    return std::make_tuple(stringColumnIndexes, numericColumnIndexes, colorColumnIndexes);
}



QJsonObject mapToJson(const std::map<QString, std::map<QString, std::map<QString, QString>>>& map) {
    QJsonObject obj;
    for (const auto& pair : map) {
        QJsonObject innerObj;
        for (const auto& innerPair : pair.second) {
            QJsonObject innerInnerObj;
            for (const auto& innerInnerPair : innerPair.second) {
                innerInnerObj[innerInnerPair.first] = innerInnerPair.second;
            }
            innerObj[innerPair.first] = innerInnerObj;
        }
        obj[pair.first] = innerObj;
    }
    return obj;
}

// Function to convert std::map to JSON formatted string
QString mapToJsonString(const std::map<QString, std::map<QString, std::map<QString, QString>>>& map) {
    QJsonObject obj = mapToJson(map);
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}
void XSCLoaderPlugin::dialogClosedCSV(QString dataSetName, QString typeName,QString leafColumn)
{


    if (checkTypeValue ==  "AllData" || checkTypeValue == "MetaData")
    {
        QStringList headerColumnNames = _loadedData[0];
        if (typeName == "Normal")
        {
            //qDebug() << dataSetName;
            auto columnIndexes = getColumnIndexes();
            std::vector<int> stringColumnIndexes = columnIndexes.first;
            std::vector<int> numericColumnIndexes = columnIndexes.second;
            std::vector<int> pointData;
            std::vector<QString>   dimensionNames;
            for (int i = 0; i < numericColumnIndexes.size(); i++)
            {
                std::vector<float> numericValues = extractNumericColumnValues(numericColumnIndexes[i]);
                for (int j = 0; j < numericValues.size(); j++)
                {
                    pointData.push_back(numericValues[j]);
                }
                dimensionNames.push_back(headerColumnNames[numericColumnIndexes[i]]);
            }
            Dataset<Points> pointValuesDataset = mv::data().createDataset("Points", dataSetName);
            events().notifyDatasetAdded(pointValuesDataset);
            pointValuesDataset->setData(pointData.data(), pointData.size() / dimensionNames.size(), dimensionNames.size());
            pointValuesDataset->setDimensionNames(dimensionNames);
            events().notifyDatasetDataChanged(pointValuesDataset);


            for (int i = 0; i < stringColumnIndexes.size(); i++)
            {
                QString clusterDatasetName = headerColumnNames[stringColumnIndexes[i]];
                std::vector<QString> stringValues = extractStringColumnValues(stringColumnIndexes[i]);
                std::map<QString, std::vector<unsigned>> clusterDetailsContainer;
                for (unsigned j = 0; j < stringValues.size(); ++j) {
                    clusterDetailsContainer[stringValues[j]].push_back(j);
                }
                Dataset<Clusters> clusterValuesDataset = mv::data().createDataset("Cluster", clusterDatasetName, pointValuesDataset);
                events().notifyDatasetAdded(clusterValuesDataset);
                for (const auto& pair : clusterDetailsContainer) {
                    Cluster cluster;
                    cluster.setName(pair.first);
                    cluster.setIndices(pair.second);
                    cluster.setColor(QColor(Qt::gray));
                    clusterValuesDataset->addCluster(cluster);
                }
                events().notifyDatasetDataChanged(clusterValuesDataset);
            }

        }
        else if (typeName == "Meta")
        {
            //qDebug() << leafColumn;
            //qDebug() << dataSetName;
            int columnNIndex=0;
            std::map <QString, std::pair<QString, QString>> metaData;
            for (int i = 0; i < headerColumnNames.size(); i++)
            {
                if (headerColumnNames[i] == leafColumn)
                {
                    columnNIndex=i;
                    break;
                }

            }

            std::vector<int> stringColumnIndexes, numericColumnIndexes, colorColumnIndexes;
            std::tie(stringColumnIndexes, numericColumnIndexes, colorColumnIndexes) = getValueIndexes();
            std::vector<QString> keyValues= extractStringColumnValues(columnNIndex);



            std::map<QString, std::map<QString, std::map<QString,QString>>> metaDataMap;

            for (int i = 0; i < keyValues.size(); i++)
            {
                
                if (stringColumnIndexes.size() > 0) {
                    std::map<QString, QString> tempMapInnerString;

                    for (int j = 0; j < stringColumnIndexes.size(); j++)
                    {

                        tempMapInnerString[headerColumnNames[stringColumnIndexes[j]]] = _loadedData[i + 1][stringColumnIndexes[j]];
                    }
                    metaDataMap[keyValues[i]]["String"] = tempMapInnerString;
                }


                if (colorColumnIndexes.size() > 0) {
                    std::map<QString,QString> tempMapInnerColor;

                    for (int j = 0; j < colorColumnIndexes.size(); j++)
                    {

                        tempMapInnerColor[headerColumnNames[colorColumnIndexes[j]]] = _loadedData[i + 1][colorColumnIndexes[j]];
                    }
                    metaDataMap[keyValues[i]]["Color"] = tempMapInnerColor;
                }

                ;
                if (numericColumnIndexes.size() > 0) {
                    std::map<QString, QString> tempMapInnerNumeric;

                    for (int j = 0; j < numericColumnIndexes.size(); j++)
                    {

                        tempMapInnerNumeric[headerColumnNames[numericColumnIndexes[j]]] = _loadedData[i + 1][numericColumnIndexes[j]];
                    }
                    metaDataMap[keyValues[i]]["Numeric"] = tempMapInnerNumeric;

                }

            }


            QString jsonString = mapToJsonString(metaDataMap);
            //qDebug() << "JSON String:" << jsonString;
            Dataset<XSCTreeMeta> metaValuesDataset = mv::data().createDataset("XSCTreeMeta", dataSetName);
            events().notifyDatasetAdded(metaValuesDataset);


            QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

            if (!doc.isNull())
            {
                if (doc.isObject())
                {
                    QJsonObject jsonObj = doc.object();
                    metaValuesDataset->setTreeMetaData(jsonObj);
                    events().notifyDatasetDataChanged(metaValuesDataset);
                    // Now jsonObj holds your JSON object
                }
                else
                {
                    qDebug() << "Document is not an object";
                }
            }
            else
            {
                qDebug() << "Invalid JSON...\n";// << jsonString;
            }


            



        }

    }

    /*
    identifierDatasetName = dataSetName;

    std::string realType= typeName.toStdString();

    if(checkTypeValue == "XSCTree" && realType == "XSCTree")
    {
        if (checkTypeValue == "XSCTree")
        {



            pointValuesDatasetName = identifierDatasetName + "_Tree";



        }


        Dataset<Points> pointValuesDataset = mv::data().createDataset("Points", pointValuesDatasetName );

        events().notifyDatasetAdded(pointValuesDataset);


        pointValuesDataset->setData(pointValuesDatasetDatasetDataValues.data(), pointValuesDatasetDatasetNumPoints, pointValuesDatasetDatasetNumDimensions);
        pointValuesDataset->setDimensionNames(pointValuesDatasetDatasetDimensionNames);


        events().notifyDatasetDataChanged(pointValuesDataset);
    }
    else
    {
        if (checkTypeValue == "Meta" || (checkTypeValue == "XSCTree"))
        {
            _numericColumnValues.clear();
            _stringColumnStrings.clear();
            _columnHeaderNameValues.clear();
            pointValuesDatasetDatasetDataValues.clear();

            // Call the method to store columns
            storeColumns(_loadedData, _numericColumnValues, _stringColumnStrings, _columnHeaderNameValues);

            pointValuesDatasetName = identifierDatasetName + "_Metadata";

            Dataset<Points> pointValuesDataset = mv::data().createDataset("Points", pointValuesDatasetName);
            events().notifyDatasetAdded(pointValuesDataset);
            pointValuesDatasetDatasetDimensionNames.clear();
            // Iterate over the map using a range-based for loop
            for (const auto& pair : _numericColumnValues)
            {
                int key = pair.first;  // Access the key

                if (_columnHeaderNameValues.find(key) != _columnHeaderNameValues.end()) {
                    std::string value = _columnHeaderNameValues[key];
                    pointValuesDatasetDatasetDimensionNames.push_back(QString::fromStdString(value));
                }
                else {
                    pointValuesDatasetDatasetDimensionNames.push_back("NotAvailabe");
                }

                const std::vector<float>& values = pair.second;  // Access the vector of floats
                // Output vector of floats
                for (const auto& value : values) {
                    pointValuesDatasetDatasetDataValues.push_back(value);
                }


            }
            pointValuesDatasetDatasetNumDimensions = pointValuesDatasetDatasetDimensionNames.size();
            pointValuesDatasetDatasetNumPoints = pointValuesDatasetDatasetDataValues.size() / pointValuesDatasetDatasetDimensionNames.size();


            pointValuesDataset->setData(pointValuesDatasetDatasetDataValues.data(), pointValuesDatasetDatasetNumPoints, pointValuesDatasetDatasetNumDimensions);
            pointValuesDataset->setDimensionNames(pointValuesDatasetDatasetDimensionNames);
            events().notifyDatasetDataChanged(pointValuesDataset);


                // Iterate through the map

            for (const auto& pair : _stringColumnStrings) {

                int key = pair.first;
                const std::vector<std::string>& value = pair.second;
                std::map<std::string, std::vector<unsigned>> tempClusterDetailsContainer;

                if (_columnHeaderNameValues.find(key) != _columnHeaderNameValues.end()) {
                    std::string value = _columnHeaderNameValues[key];
                    clusterValuesDatasetName = identifierDatasetName + "_" + QString::fromStdString(value);
                }
                else {
                    clusterValuesDatasetName = identifierDatasetName + "_" + "NotAvailabe";
                }
                Dataset<Clusters>  clusterValuesDataset = mv::data().createDataset("Cluster", clusterValuesDatasetName, pointValuesDataset);

                events().notifyDatasetAdded(clusterValuesDataset);

                for (unsigned i = 0; i < value.size(); ++i) {
                    tempClusterDetailsContainer[value[i]].push_back(i);
                }

                for (const auto& pair : tempClusterDetailsContainer)
                {
                    Cluster cluster;
                    cluster.setName(QString::fromStdString(pair.first));
                    cluster.setIndices(pair.second);
                    cluster.setColor(QColor(Qt::gray));
                    clusterValuesDataset->addCluster(cluster);
                }
                events().notifyDatasetDataChanged(clusterValuesDataset);


            }


        }
    }

    */

}


XSCLoaderPluginFactory::XSCLoaderPluginFactory()
{
    setIconByName("upload");
}

XSCLoaderPlugin* XSCLoaderPluginFactory::produce()
{
    return new XSCLoaderPlugin(this);
}

mv::DataTypes XSCLoaderPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    supportedTypes.append(ClusterType);
    return supportedTypes;
}
