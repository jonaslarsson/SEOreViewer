#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QVector>
#include <QMap>

struct MapStatistics
{
    QMap<int,int> oreCount;
    QMap<int,int> materialCount;
};

class MapWidget : public QWidget
{
    Q_OBJECT
    QImage m_heightMap;
    QImage m_materialsMap;
    QMap<int,QVector<QPoint>* > m_ores;
    QMap<int,QImage*> m_materials;
    int m_highlightOre;
    int m_highlightMaterial;

public:
    explicit MapWidget(QWidget *parent = nullptr);
    void setHeightMap(const QImage& image);
    void setMaterialsMap(const QImage& image, MapStatistics *stats);
    void setHighlightOre(int ore);
    void setHighlightMaterial(int material);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

};

#endif // MAPWIDGET_H
