/****************************************************************
 * @file    Settings.cpp
 * @brief   Implementation of Settings class.
 *
 * @author  Jeffrey Scott Flesher with the help of Copilot
 * @version 0.1
 * @date    2025-11-09
 * @section License MIT
 * @section DESCRIPTION
 * Loads, saves, imports, and exports settings from/to JSON file.
 ***************************************************************/

#include "Settings.h"

Settings::Settings(const QString &jsonPath)
    : m_jsonPath(jsonPath)
{
}

bool Settings::load()
{
    QFile file(m_jsonPath);
    if (!file.exists())
    {
        m_settings = QJsonObject(); // Start fresh
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject())
        return false;
    m_settings = doc.object();
    return true;
}

bool Settings::save() const
{
    QFile file(m_jsonPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QJsonDocument doc(m_settings);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool Settings::import(const QString &importPath)
{
    QFile file(importPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject())
        return false;
    m_settings = doc.object();
    return save();
}

bool Settings::exportTo(const QString &exportPath) const
{
    QFile file(exportPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QJsonDocument doc(m_settings);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    if (m_settings.contains(key))
        return m_settings.value(key).toVariant();
    return defaultValue;
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    QJsonValue jsonVal = QJsonValue::fromVariant(value);
    m_settings.insert(key, jsonVal);
}

QJsonObject Settings::getJson() const
{
    return m_settings;
}

void Settings::setJson(const QJsonObject &obj)
{
    m_settings = obj;
}

/************** End of Settings.cpp *****************************/
