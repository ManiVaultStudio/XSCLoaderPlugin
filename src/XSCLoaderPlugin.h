#pragma once

#include <LoaderPlugin.h>
#include "InputDialogCSV.h"
#include "InputDialogJSON.h"
#include "InputDialogCSCBIN.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "XSCTreeData/XSCTreeData.h"
#include "XSCTreeMetaData/XSCTreeMetaData.h"

#include <QRandomGenerator>
#include <QtMath>
#include <fstream>
#include <iostream>
#include <set>
using namespace mv::plugin;
struct DataMain
{
    int rows;
    int columns;
    std::vector<float> values;
    std::vector<std::string> dimensionNames;
    std::string  mainDatasetName;
};

struct SingleClusterContainer
{
    std::string clusterName;
    std::string clusterColor;
    std::vector<int> clusterIndices;
};

struct DataClusterForADataset
{
    std::vector<SingleClusterContainer> clusterValues;
    std::string                         derivedclusterdatasetName;
};

struct DataPointsDerived
{
    int rows;
    int columns;
    std::vector<float> values;
    std::vector<std::string> dimensionNames;
    std::string derivedpointdatasetName;
};

struct BinSet
{
    DataMain dataMain;
    std::vector<DataClusterForADataset> dataClustersDerived;
    std::vector<DataPointsDerived> dataPointsDerived;
};

// =============================================================================
// Loader
// =============================================================================
struct ClusterInput {
    std::string value1;
    std::string value2;
    std::string cluster;
};
class XSCLoaderPlugin : public LoaderPlugin
{
    Q_OBJECT
public:
    XSCLoaderPlugin(const PluginFactory* factory);
    ~XSCLoaderPlugin(void) override;
    
    void init() override;

    void loadData() Q_DECL_OVERRIDE;

    void saveBinSet(const BinSet& binSet, const std::string& filename);
    std::pair<BinSet, QString> readBinSet(const std::string& filename);


public slots:
    void dialogClosedCSV(QString dataSetName, QString TypeName, QString leafColumn);
    void dialogClosedJSON(QString dataSetName, QString TypeName);
    void dialogClosedCSCBIN(QString dataSetName, QString TypeName);
    std::pair<std::vector<int>, std::vector<int>> getColumnIndexes();
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> getValueIndexes();
    std::vector<QString> extractStringColumnValues(int columnIndex);
    std::vector<float> extractNumericColumnValues(int columnIndex);

private:
    QString _dataSetName;
    std::vector<QStringList> _loadedData;
    std::string checkTypeValue = "None";
    std::map<int, std::vector<float>> _numericColumnValues;
    std::map<int, std::vector<std::string>> _stringColumnStrings;
    std::map<int, std::string> _columnHeaderNameValues;

    std::vector<QString> _speciesNames;
    std::vector<ClusterInput> _speciesOrder;
    QString pointValuesDatasetName;
    int pointValuesDatasetDatasetNumPoints;
    int pointValuesDatasetDatasetNumDimensions;
    std::vector<QString>  pointValuesDatasetDatasetDimensionNames;
    std::vector<QString>  clusterValuesDatasetDatasetClusterNames;
    std::vector<float> pointValuesDatasetDatasetDataValues;
    QString clusterValuesDatasetName;
    QString identifierDatasetName;
    std::vector<float> identifierDatasetCell;
    std::vector<QString> identifierDatasetCellDimensionNames;
    QJsonObject _treeData;  
    BinSet    _binSetRead;
};


// =============================================================================
// Factory
// =============================================================================

class XSCLoaderPluginFactory : public LoaderPluginFactory
{
    Q_INTERFACES(mv::plugin::LoaderPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.XSCLoaderPlugin"
                      FILE  "XSCLoaderPlugin.json")
    
public:
    XSCLoaderPluginFactory(void);
    ~XSCLoaderPluginFactory(void) override {}
    XSCLoaderPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;
};
