#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "DataTypes.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QDomElement;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_browseDefFile_clicked();
    void on_browseHeightMap_clicked();
    void on_browseMaterialsMap_clicked();
    void on_comboBoxPlanetSelection_currentIndexChanged(int index);

    void on_tableWidgetOres_itemSelectionChanged();

private:
    QVector<Planet> m_planets;
    bool parsePlanet(const QDomElement& rootElement, Planet *planet);
    void openDefinitionsFile(const QString& filename);
    void openHeightMapFile(const QString& filename);
    void openMaterialsMapFile(const QString& filename);
    Ui::MainWindow *m_gui;
};
#endif // MAINWINDOW_H
