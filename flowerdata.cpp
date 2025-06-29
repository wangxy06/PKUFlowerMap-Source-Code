#include "flowerdata.h"
#include <ActiveQt/QAxObject>
#include <QFile>
#include <QDir>
#include <QDebug>

void Flower::loadFromExcel()
{
    qDebug() << "开始硬编码导入花卉数据";
    qDebug() << "成功加载" << m_flowers.size() << "条花卉数据";
}

QVector<const FlowerInfo*> Flower::getFlowersByLocation(const QString& location) const
{
    QVector<const FlowerInfo*> result;
    for (const auto& flower : m_flowers) {
        if (flower.locationNames.contains(location)) {
            result.append(&flower);
        }
    }
    return result;
}

// LocationManager 实现
void LocationManager::addLocation(const QString& name, const QPointF& coord)
{
    m_locationMap[name] = coord;
}

QPointF LocationManager::getLocation(const QString& name) const
{
    return m_locationMap.value(name, QPointF());
}

QStringList LocationManager::getAllLocationNames() const
{
    return m_locationMap.keys();
}
