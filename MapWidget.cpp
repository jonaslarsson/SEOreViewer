#include "MapWidget.h"
#include <QImage>
#include <QPaintEvent>
#include <QPainter>

MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    m_highlightOre = 255;
}

void MapWidget::setHeightMap(const QImage &image)
{
    m_heightMap = image;
    repaint();
}

void MapWidget::setMaterialsMap(const QImage &image)
{
    m_materialsMap = image;
    foreach (QVector<QPoint>* ore, m_ores)
    {
        delete ore;
    }
    m_ores.clear();
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
            }
        }
    }
    repaint();
}

void MapWidget::setHighlightOre(int ore)
{
    m_highlightOre = ore;
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
