#include "MapWidget.h"
#include <QImage>
#include <QPaintEvent>
#include <QPainter>

MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    m_highlightMaterial = -1;
    m_highlightOre = 255;
}

void MapWidget::setHeightMap(const QImage &image)
{
    m_heightMap = image;
    repaint();
}

void MapWidget::setMaterialsMap(const QImage &image, MapStatistics *stats)
{
    if (stats == nullptr) return;

    stats->materialCount.clear();
    stats->oreCount.clear();

    m_materialsMap = image;
    foreach (QVector<QPoint>* ore, m_ores)
    {
        delete ore;
    }
    m_ores.clear();
    m_materials.clear();

    for (int y = 0; y < m_materialsMap.height(); y++)
    {
        for (int x = 0; x < m_materialsMap.width(); x++)
        {
            QRgb rgb = m_materialsMap.pixel(x, y);
            quint8 ore = rgb & 0xFF;
            if (ore != 255)
            {
                if (m_ores.contains(ore) == false)
                {
                    m_ores.insert(ore, new QVector<QPoint>);
                }
                m_ores.value(ore)->append(QPoint(x,y));

                if (stats->oreCount.contains(ore) == false)
                {
                    stats->oreCount.insert(ore, 0);
                }
                stats->oreCount[ore]++;
            }

            quint8 material = rgb >> 16;
            if (m_materials.contains(material) == false)
            {
                QImage *materialImage = new QImage(image.size(), QImage::Format_ARGB32);
                materialImage->fill(Qt::transparent);
                m_materials.insert(material, materialImage);
            }
            m_materials.value(material)->setPixelColor(x, y, QColor(0,0,255,64));

            if (stats->materialCount.contains(material) == false)
            {
                stats->materialCount.insert(material, 0);
            }
            stats->materialCount[material]++;
        }
    }
    repaint();
}

void MapWidget::setHighlightOre(int ore)
{
    m_highlightOre = ore;
    repaint();
}

void MapWidget::setHighlightMaterial(int material)
{
    m_highlightMaterial = material;
    repaint();
}

void MapWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.save();

    int side = qMin(width(), height());
    qreal scale = (qreal)side / (qreal)m_heightMap.width();

    painter.scale(scale, scale);

    QRectF rect(0, 0, m_heightMap.width(), m_heightMap.height());

    painter.drawImage(rect, m_heightMap);

    if (m_materials.contains(m_highlightMaterial))
    {
        painter.drawImage(rect, *m_materials.value(m_highlightMaterial));
    }

    QPen pen;
    pen.setWidth(2);
    pen.setBrush(Qt::green);
    painter.setPen(pen);

    if (m_ores.contains(m_highlightOre))
    {
        QVector<QPoint>* ore = m_ores.value(m_highlightOre);
        painter.drawPoints(ore->constData(), ore->count());
    }


    painter.restore();
}
