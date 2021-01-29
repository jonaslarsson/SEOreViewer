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

struct Planet
{
    QString name;
    QVector<Ore> ores;
};

#endif // DATATYPES_H
