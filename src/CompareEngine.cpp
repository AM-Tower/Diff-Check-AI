/******************************************************************************
 * @file CompareEngine.cpp
 * @brief Implements the CompareEngine class for code comparison.
 *
 * @author Jeffrey Scott Flesher with the help of AI: Copilot
 * @version 0.8
 * @date 2025-11-07
 * @section License MIT
 * @section DESCRIPTION
 * Regex-based function extraction, normalization (whitespace/
 * brace compression), and a simple diff algorithm that marks
 * deletions (-), additions (+), and reorders (~) heuristically.
 ******************************************************************************/

#include "CompareEngine.h"
#include <QRegularExpression>
#include <utility> // for std::as_const

/******************************************************************************
 * @brief Constructor.
 ******************************************************************************/
CompareEngine::CompareEngine()
{
}

/******************************************************************************
 * @brief Destructor.
 ******************************************************************************/
CompareEngine::~CompareEngine()
{
}

/******************************************************************************
 * @brief Removes comments from code.
 * @param code Raw file content.
 * @return Code without comments.
 ******************************************************************************/
QString CompareEngine::stripComments(const QString &code) const
{
    QString s = code;
    QRegularExpression blockComments(R"(/\*.*?\*/)", QRegularExpression::DotMatchesEverythingOption);
    s.remove(blockComments);
    QRegularExpression lineComments(R"(//[^\n]*)");
    s = s.replace(lineComments, "");
    return s;
}

/******************************************************************************
 * @brief Extracts function blocks from raw C/C++ code.
 * @param code Raw file content.
 * @return Map of function name to FunctionBlock.
 ******************************************************************************/
QMap<QString, FunctionBlock> CompareEngine::extractFunctions(const QString &code) const
{
    QMap<QString, FunctionBlock> out;
    const QString s = stripComments(code);
    QRegularExpression re(R"(([\w:~]+)\s*\([^\)]*\)\s*\{)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(s);
    while (it.hasNext())
    {
        QRegularExpressionMatch m = it.next();
        int idx = m.capturedStart(0);
        if (idx < 0) { continue; }
        int braceStart = s.indexOf('{', idx);
        if (braceStart < 0) { continue; }
        int depth = 0;
        int pos = braceStart;
        int endPos = -1;
        while (pos < s.size())
        {
            QChar c = s.at(pos);
            if (c == '{') { depth++; }
            else if (c == '}')
            {
                depth--;
                if (depth == 0)
                {
                    endPos = pos;
                    break;
                }
            }
            pos++;
        }
        if (endPos < 0) { continue; }
        QString signature = s.mid(idx, braceStart - idx).trimmed();
        QString name = m.captured(1).trimmed();
        QString body = s.mid(braceStart + 1, endPos - braceStart - 1);
        FunctionBlock fb;
        fb.signature = signature;
        fb.body = body;
        fb.normalizedBody = normalizeBody(body);
        fb.bodyLines = toLines(body);
        out.insert(name, fb);
    }
    return out;
}

/******************************************************************************
 * @brief Normalizes code body to ignore style/spacing.
 * @param body Raw function body.
 * @return Normalized single-line body.
 ******************************************************************************/
QString CompareEngine::normalizeBody(const QString &body) const
{
    QString s = body;
    s.replace("\r", "");
    s.replace("\t", " ");
    s = s.replace(QRegularExpression(R"(\s+)"), " ");
    s = s.replace(QRegularExpression(R"(\s*\{\s*)"), "{");
    s = s.replace(QRegularExpression(R"(\s*\}\s*)"), "}");
    return s.trimmed();
}

/******************************************************************************
 * @brief Splits text into lines.
 * @param text Source text.
 * @return Lines split by '\n'.
 ******************************************************************************/
QStringList CompareEngine::toLines(const QString &text) const
{
    QString t = text;
    t.replace("\r", "");
    return t.split('\n', Qt::KeepEmptyParts);
}

/******************************************************************************
 * @brief Computes line-by-line diff between original and new.
 * @param originalLines Lines from original function body.
 * @param newLines Lines from new function body.
 * @return Triples of (marker, originalText, newText).
 ******************************************************************************/
QVector<QVector<QString>> CompareEngine::diffLines(const QStringList &originalLines,
                                                   const QStringList &newLines) const
{
    auto norm = [](const QString &l) -> QString {
        QString s = l;
        s.replace("\t", " ");
        s = s.replace(QRegularExpression(R"(\s+)"), " ").trimmed();
        return s;
    };
    int n = originalLines.size();
    int m = newLines.size();
    QVector<QVector<int>> dp(n + 1, QVector<int>(m + 1, 0));
    for (int i = n - 1; i >= 0; --i)
    {
        for (int j = m - 1; j >= 0; --j)
        {
            if (norm(originalLines[i]) == norm(newLines[j]))
            {
                dp[i][j] = 1 + dp[i + 1][j + 1];
            }
            else
            {
                dp[i][j] = qMax(dp[i + 1][j], dp[i][j + 1]);
            }
        }
    }
    QVector<QVector<QString>> result;
    int i = 0, j = 0;
    while (i < n && j < m)
    {
        if (norm(originalLines[i]) == norm(newLines[j]))
        {
            result.push_back({ " ", originalLines[i], newLines[j] });
            i++; j++;
        }
        else if (dp[i + 1][j] >= dp[i][j + 1])
        {
            result.push_back({ "-", originalLines[i], "" });
            i++;
        }
        else
        {
            result.push_back({ "+", "", newLines[j] });
            j++;
        }
    }
    while (i < n) { result.push_back({ "-", originalLines[i], "" }); i++; }
    while (j < m) { result.push_back({ "+", "", newLines[j] }); j++; }
    for (int k = 0; k < result.size(); ++k)
    {
        if (result[k][0] == "+")
        {
            QString added = norm(result[k][2]);
            for (int r = 0; r < result.size(); ++r)
            {
                if (result[r][0] == "-" && norm(result[r][1]) == added)
                {
                    result[k][0] = "~"; // reorder
                    break;
                }
            }
        }
    }
    return result;
}

/******************************************************************************
 * @brief Builds textual summary of missing/new/changed functions.
 * @param orig Map of original functions.
 * @param news Map of new functions.
 * @return Multi-line summary text.
 ******************************************************************************/
QString CompareEngine::buildSummary(const QMap<QString, FunctionBlock> &orig,
                                    const QMap<QString, FunctionBlock> &news) const
{
    QStringList lines;
    lines << "=== Summary ===";
    QStringList missingFuncs;
    QStringList newFuncs;
    QStringList changedFuncs;
    for (auto it = orig.constBegin(); it != orig.constEnd(); ++it)
    {
        const QString &name = it.key();
        if (!news.contains(name))
        {
            missingFuncs << name;
        }
        else if (it.value().normalizedBody != news.value(name).normalizedBody)
        {
            changedFuncs << name;
        }
    }
    for (auto it = news.constBegin(); it != news.constEnd(); ++it)
    {
        const QString &name = it.key();
        if (!orig.contains(name))
        {
            newFuncs << name;
        }
    }
    lines << QString("Missing functions in new (%1):").arg(missingFuncs.size());
    for (const QString &f : std::as_const(missingFuncs))
    {
        lines << " - " + f;
    }
    lines << QString("New functions not in original (%1):").arg(newFuncs.size());
    for (const QString &f : std::as_const(newFuncs))
    {
        lines << " + " + f;
    }
    lines << QString("Changed function bodies (%1):").arg(changedFuncs.size());
    for (const QString &f : std::as_const(changedFuncs))
    {
        lines << " * " + f;
    }
    return lines.join("\n");
}

/*************** End of CompareEngine.cpp ************************************/
