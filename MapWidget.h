#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QVector>
#include <QMap>

class MapWidget : public QWidget
{
    Q_OBJECT
    QImage m_heightMap;
    QImage m_materialsMap;
    QMap<int,QVector<QPoint>* > m_ores;
    int m_highlightOre;

public:
    explicit MapWidget(QWidget *parent = nullptr);
    void setHeightMap(const QImage& image);
    void setMaterialsMap(const QImage& image);
    void setHighlightOre(int ore);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

};

#endif // MAPWIDGET_H
