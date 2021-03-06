#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DataTypes.h"
#include <QFileDialog>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDomDocument>
#include <QClipboard>

namespace
{
enum SeFileType
{
    InvalidFile,
    DefinitionsFile,
    HeightMapFile,
    MaterialsMapFile
};

SeFileType fileType(const QString& filename)
{
    if (filename.endsWith("_mat.png")) return MaterialsMapFile;
    if (filename.endsWith(".png")) return HeightMapFile;
    if (filename.endsWith(".sbc"))
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) return InvalidFile;
        QDomDocument sbc;
        if (!sbc.setContent(&file)) return InvalidFile;

        QDomElement rootElement = sbc.documentElement();
        if (rootElement.tagName() != "Definitions") return InvalidFile;

        QDomElement child = rootElement.firstChildElement("Definition");
        if (child.isNull() == false)
        {
            if (child.attribute("xsi:type") == "PlanetGeneratorDefinition") return DefinitionsFile;
            return InvalidFile;
        }

        child = rootElement.firstChildElement("PlanetGeneratorDefinitions");
        if (child.tagName() == "PlanetGeneratorDefinitions") return DefinitionsFile;
        return InvalidFile;
    }

    return InvalidFile;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_gui(new Ui::MainWindow)
{
    m_gui->setupUi(this);
    m_gui->tableWidgetOres->setContextMenuPolicy(Qt::CustomContextMenu);
}

MainWindow::~MainWindow()
{
    delete m_gui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QList<SeFileType> typesFound;
        foreach (const QUrl& url, event->mimeData()->urls())
        {
            if (!url.isLocalFile()) return;
            QString file = url.toLocalFile();
            SeFileType type = fileType(file);
            if (type == InvalidFile) return;
            if (typesFound.contains(type)) return;
            typesFound.append(type);
        }
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    foreach (const QUrl& url, event->mimeData()->urls())
    {
        QString filename = url.toLocalFile();
        if (filename.endsWith("_mat.png"))
        {
            openMaterialsMapFile(filename);
        }
        else if (filename.endsWith(".png"))
        {
            openHeightMapFile(filename);
        }
        else
        {
            openDefinitionsFile(filename);
        }
    }
    event->acceptProposedAction();
}

void MainWindow::on_browseDefFile_clicked()
{
    QString dir;
    QString filter = "Planet definition (*.sbc)";
    QString filename = QFileDialog::getOpenFileName(this, "Open planet definition", dir, filter);
    if (filename.isNull() == false)
    {
        openDefinitionsFile(filename);
    }
}

void MainWindow::on_browseHeightMap_clicked()
{
    QString dir;
    QString filter = "Height map (*.png)";
    QString filename = QFileDialog::getOpenFileName(this, "Open height map", dir, filter);
    if (filename.isNull() == false && filename.endsWith("_mat.png") == false)
    {
        openHeightMapFile(filename);
    }
}

void MainWindow::on_browseMaterialsMap_clicked()
{
    QString dir;
    QString filter = "Materials map (*_mat.png)";
    QString filename = QFileDialog::getOpenFileName(this, "Open materials map", dir, filter);
    if (filename.isNull() == false)
    {
        openMaterialsMapFile(filename);
    }
}

bool MainWindow::parsePlanet(const QDomElement &rootElement, Planet *planet)
{
    QDomElement id = rootElement.firstChildElement("Id");
    QDomElement typeId = id.firstChildElement("TypeId");
    QDomElement subtypeId = id.firstChildElement("SubtypeId");

    if (typeId.text() != "PlanetGeneratorDefinition") return false;
    planet->name = subtypeId.text();
    if (planet->name.size() == 0) return false;

    QDomElement oreMappings = rootElement.firstChildElement("OreMappings");
    if (oreMappings.isNull()) return false;

    QDomNode node = oreMappings.firstChild();
    while (node.isNull() == false)
    {
        if (node.isElement())
        {
            QDomElement element = node.toElement();
            if (element.tagName() != "Ore") return false;
            QString value = element.attribute("Value");
            QString type = element.attribute("Type");
            QString start = element.attribute("Start");
            QString depth = element.attribute("Depth");

            if (value.isEmpty()) return false;
            if (type.isEmpty()) return false;
            if (start.isEmpty()) return false;
            if (depth.isEmpty()) return false;

            bool ok;
            Ore ore;
            ore.type = type;
            ore.value = value.toInt(&ok);
            if (!ok) return false;
            ore.start = start.toInt(&ok);
            if (!ok) return false;
            ore.depth = depth.toInt(&ok);
            if (!ok) return false;

            planet->ores.append(ore);
        }

        node = node.nextSibling();
    }

    QDomElement materials = rootElement.firstChildElement("ComplexMaterials");
    if (materials.isNull()) return false;
    node = materials.firstChild();
    while (node.isNull() == false)
    {
        if (node.isElement())
        {
            QDomElement element = node.toElement();
            if (element.tagName() != "MaterialGroup") return false;
            QString name = element.attribute("Name");
            QString value = element.attribute("Value");

            bool ok;
            Material material;
            material.name = name;
            material.value = value.toInt(&ok);
            if (!ok) return false;

            planet->materials.append(material);
        }
        node = node.nextSibling();
    }

    return true;
}

void MainWindow::openDefinitionsFile(const QString &filename)
{
    if (filename.endsWith(".sbc") == false) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    QDomDocument sbc;
    if (!sbc.setContent(&file)) return;

    QDomElement rootElement = sbc.documentElement();
    if (rootElement.tagName() != "Definitions") return;

    QVector<Planet> planets;

    QDomElement child = rootElement.firstChildElement("Definition");
    if (child.isNull() == false)
    {
        if (child.attribute("xsi:type") != "PlanetGeneratorDefinition") return;

        // Parse multi planet file
        QDomNode node = rootElement.firstChild();
        while (node.isNull() == false)
        {
            if (node.isElement())
            {
                QDomElement element = node.toElement();
                if (element.tagName() == "Definition" && element.attribute("xsi:type") == "PlanetGeneratorDefinition")
                {
                    Planet planet;
                    if (parsePlanet(element, &planet))
                    {
                        planets.append(planet);
                    }
                }
            }

            node = node.nextSibling();
        }
    }
    else
    {
        child = rootElement.firstChildElement("PlanetGeneratorDefinitions");
        if (child.tagName() == "PlanetGeneratorDefinitions")
        {
            // Parse single planet file
            QDomElement planetElement = child.firstChildElement("PlanetGeneratorDefinition");
            if (planetElement.isNull() == false)
            {
                Planet planet;
                if (parsePlanet(planetElement, &planet))
                {
                    planets.append(planet);
                }
            }
        }
    }

    if (planets.count() > 0)
    {
        m_planets.clear();
        m_planets = planets;
        m_gui->lineEditDefFile->setText(filename);
        m_gui->comboBoxPlanetSelection->clear();
        foreach (const Planet& planet, m_planets)
        {
            m_gui->comboBoxPlanetSelection->addItem(planet.name);
        }
    }
}

void MainWindow::openHeightMapFile(const QString &filename)
{
    QImage image(filename);
    if (image.isNull() == false)
    {
        m_gui->mapWidget->setHeightMap(image);
        m_gui->lineEditHeightMap->setText(filename);
    }
}

void MainWindow::openMaterialsMapFile(const QString &filename)
{
    QImage image(filename);
    if (image.isNull() == false)
    {
        m_gui->mapWidget->setMaterialsMap(image, &m_stats);
        m_gui->lineEditMaterialsMap->setText(filename);
        updateTables();
    }
}

void MainWindow::updateTables()
{
    int index = m_gui->comboBoxPlanetSelection->currentIndex();
    m_gui->tableWidgetOres->clear();
    m_gui->mapWidget->setHighlightOre(255);
    if (index >= 0 && index < m_planets.count())
    {
        const Planet& planet = m_planets.at(index);

        m_gui->tableWidgetOres->setColumnCount(3);
        m_gui->tableWidgetOres->setRowCount(planet.ores.count());

        m_gui->tableWidgetOres->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
        m_gui->tableWidgetOres->setHorizontalHeaderItem(1, new QTableWidgetItem("Type"));
        m_gui->tableWidgetOres->setHorizontalHeaderItem(2, new QTableWidgetItem("#"));

        for (int i = 0; i < planet.ores.count(); i++)
        {
            const Ore& ore = planet.ores.at(i);
            int count = m_stats.oreCount.value(ore.value, 0);
            m_gui->tableWidgetOres->setItem(i, 0, new QTableWidgetItem(QString::number(ore.value)));
            m_gui->tableWidgetOres->setItem(i, 1, new QTableWidgetItem(ore.type));
            m_gui->tableWidgetOres->setItem(i, 2, new QTableWidgetItem(QString::number(count)));
        }

        m_gui->tableWidgetMaterials->setColumnCount(3);
        m_gui->tableWidgetMaterials->setRowCount(planet.materials.count());

        m_gui->tableWidgetMaterials->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
        m_gui->tableWidgetMaterials->setHorizontalHeaderItem(1, new QTableWidgetItem("Name"));
        m_gui->tableWidgetMaterials->setHorizontalHeaderItem(2, new QTableWidgetItem("#"));

        for (int i = 0; i < planet.materials.count(); i++)
        {
            const Material& material = planet.materials.at(i);
            int count = m_stats.materialCount.value(material.value, 0);
            m_gui->tableWidgetMaterials->setItem(i, 0, new QTableWidgetItem(QString::number(material.value)));
            m_gui->tableWidgetMaterials->setItem(i, 1, new QTableWidgetItem(material.name));
            m_gui->tableWidgetMaterials->setItem(i, 2, new QTableWidgetItem(QString::number(count)));
        }
    }
}

void MainWindow::on_comboBoxPlanetSelection_currentIndexChanged(int)
{
    updateTables();
}

void MainWindow::on_tableWidgetOres_itemSelectionChanged()
{
    if (m_gui->tableWidgetOres->selectedItems().count() > 0)
    {
        int oreValue = m_gui->tableWidgetOres->selectedItems().first()->text().toInt();
        m_gui->mapWidget->setHighlightOre(oreValue);
    }
}

void MainWindow::on_tableWidgetMaterials_itemSelectionChanged()
{
    if (m_gui->tableWidgetMaterials->selectedItems().count() > 0)
    {
        int materialValue = m_gui->tableWidgetMaterials->selectedItems().first()->text().toInt();
        m_gui->mapWidget->setHighlightMaterial(materialValue);
    }
}

void MainWindow::on_tableWidgetOres_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    QAction *copyTable = new QAction("Copy entire table to clipboard", this);
    connect(copyTable, &QAction::triggered, this, &MainWindow::copyOresTableToClipboard);
    menu->addAction(copyTable);
    menu->popup(m_gui->tableWidgetOres->viewport()->mapToGlobal(pos));
}

void MainWindow::copyOresTableToClipboard()
{
    copyTableToClipboard(m_gui->tableWidgetOres);
}

void MainWindow::copyTableToClipboard(QTableWidget *table)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QStringList tableData;
    QStringList rowData;
    for (int row = 0; row < table->rowCount(); row++)
    {
        rowData.clear();
        for (int column = 0; column < table->columnCount(); column++)
        {
            rowData << table->item(row, column)->text();
        }
        tableData << rowData.join("\t");
    }

    clipboard->setText(tableData.join("\r\n"));
}
