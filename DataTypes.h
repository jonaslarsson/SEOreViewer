#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QVector>

struct Ore
{
    int value;
    QString type;
    int start;
    int depth;
};

struct Material
{
    QString name;
    int value;
};

struct Planet
{
    QString name;
    QVector<Ore> ores;
    QVector<Material> materials;
};

#endif // DATATYPES_H
