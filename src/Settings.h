/****************************************************************
 * @file    Settings.h
 * @brief   Application settings manager using JSON file.
 *
 * @author  Jeffrey Scott Flesher with the help of Copilot
 * @version 0.1
 * @date    2025-11-09
 * @section License MIT
 * @section DESCRIPTION
 * Loads, saves, imports, and exports settings from/to JSON file.
 ***************************************************************/

#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QVariant>

class Settings
{
public:
    /**************************************************************
     * @brief Constructor.
     * @param jsonPath Path to the JSON settings file.
     *************************************************************/
    explicit Settings(const QString &jsonPath);

    /**************************************************************
     * @brief Loads settings from JSON file.
     * @return true if loaded, false otherwise.
     *************************************************************/
    bool load();

    /**************************************************************
     * @brief Saves settings to JSON file.
     * @return true if saved, false otherwise.
     *************************************************************/
    bool save() const;

    /**************************************************************
     * @brief Imports settings from another JSON file.
     * @param importPath Path to import file.
     * @return true if imported, false otherwise.
     *************************************************************/
    bool import(const QString &importPath);

    /**************************************************************
     * @brief Exports settings to another JSON file.
     * @param exportPath Path to export file.
     * @return true if exported, false otherwise.
     *************************************************************/
    bool exportTo(const QString &exportPath) const;

    /**************************************************************
     * @brief Gets a value from settings.
     * @param key The key to look up.
     * @param defaultValue Value to return if key not found.
     * @return The value as QVariant.
     *************************************************************/
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /**************************************************************
     * @brief Sets a value in settings.
     * @param key The key to set.
     * @param value The value to set.
     *************************************************************/
    void setValue(const QString &key, const QVariant &value);

    /**************************************************************
     * @brief Returns the underlying QJsonObject.
     *************************************************************/
    QJsonObject getJson() const;

    /**************************************************************
     * @brief Sets the underlying QJsonObject.
     *************************************************************/
    void setJson(const QJsonObject &obj);

private:
    QString m_jsonPath; ///< Path to JSON file
    QJsonObject m_settings; ///< Settings data
};

/************** End of Settings.h *******************************/
