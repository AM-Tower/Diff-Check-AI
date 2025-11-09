/******************************************************************************
 * @file CompareEngine.h
 * @brief Declares the CompareEngine class for code comparison.
 *
 * @author Jeffrey Scott Flesher with the help of AI: Copilot
 * @version 0.8
 * @date 2025-11-07
 * @section License MIT
 * @section DESCRIPTION
 * Provides function-aware code comparison, normalization, and diff.
 ******************************************************************************/

#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>

/******************************************************************************
 * @struct FunctionBlock
 * @brief Holds function signature, body, and normalized body.
 ******************************************************************************/
struct FunctionBlock
{
    QString signature;      /**< Function signature */
    QString body;           /**< Raw function body */
    QString normalizedBody; /**< Normalized body for comparison */
    QStringList bodyLines;  /**< Body split into lines */
};

/******************************************************************************
 * @class CompareEngine
 * @brief Implements function-aware code comparison.
 ******************************************************************************/
class CompareEngine
{
public:
    /**************************************************************************
     * @brief Constructor.
     *************************************************************************/
    CompareEngine();

    /**************************************************************************
     * @brief Destructor.
     *************************************************************************/
    ~CompareEngine();

    /**************************************************************************
     * @brief Removes comments from code.
     * @param code Raw file content.
     * @return Code without comments.
     *************************************************************************/
    QString stripComments(const QString &code) const;

    /**************************************************************************
     * @brief Extracts function blocks from raw C/C++ code.
     * @param code Raw file content.
     * @return Map of function name to FunctionBlock.
     *************************************************************************/
    QMap<QString, FunctionBlock> extractFunctions(const QString &code) const;

    /**************************************************************************
     * @brief Normalizes code body to ignore style/spacing.
     * @param body Raw function body.
     * @return Normalized single-line body.
     *************************************************************************/
    QString normalizeBody(const QString &body) const;

    /**************************************************************************
     * @brief Splits text into lines.
     * @param text Source text.
     * @return Lines split by '\n'.
     *************************************************************************/
    QStringList toLines(const QString &text) const;

    /**************************************************************************
     * @brief Computes line-by-line diff between original and new.
     * @param originalLines Lines from original function body.
     * @param newLines Lines from new function body.
     * @return Triples of (marker, originalText, newText).
     *************************************************************************/
    QVector<QVector<QString>> diffLines(const QStringList &originalLines,
                                        const QStringList &newLines) const;

    /**************************************************************************
     * @brief Builds textual summary of missing/new/changed functions.
     * @param orig Map of original functions.
     * @param news Map of new functions.
     * @return Multi-line summary text.
     *************************************************************************/
    QString buildSummary(const QMap<QString, FunctionBlock> &orig,
                         const QMap<QString, FunctionBlock> &news) const;
};

/*************** End of CompareEngine.h **************************************/
