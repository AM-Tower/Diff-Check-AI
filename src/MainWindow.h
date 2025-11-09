/******************************************************************************
 * @file MainWindow.h
 * @brief Declares the main application window for CodeHelpAI.
 *
 * @author Jeffrey Scott Flesher with the help of AI: Copilot
 * @version 0.10
 * @date    2025-11-08
 * @section License MIT
 * @section DESCRIPTION
 * Main GUI window with tabs for Compare, Settings, Temp, and Projects.
 * Integrates CompareEngine for function-aware code comparison.
 * The Compare panel is cleared and shows a progress bar during comparison.
 * Diff output is fully colorized: red for removed, green for new,
 * magenta for reordered, black for unchanged.
 ******************************************************************************/

#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QTabWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>
#include <QTreeView>
#include <QFileSystemModel>
#include <QListWidget>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QProgressBar>
#include <functional>
#include "CompareEngine.h"

/******************************************************************************
 * @class MainWindow
 * @brief Implements the main application window.
 ******************************************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /******************************************************************************
     * @brief Constructs the MainWindow.
     * @param parent Parent widget.
     ******************************************************************************/
    explicit MainWindow(QWidget *parent = nullptr);

    /******************************************************************************
     * @brief Destructor for MainWindow.
     ******************************************************************************/
    ~MainWindow();

private slots:
    /******************************************************************************
     * @brief Opens a file into the Original editor.
     ******************************************************************************/
    void actionOpenOriginal();

    /******************************************************************************
     * @brief Opens a file into the New editor.
     ******************************************************************************/
    void actionOpenNew();

    /******************************************************************************
     * @brief Saves comparison output to a file with overwrite warn.
     ******************************************************************************/
    void actionSaveComparison();

    /******************************************************************************
     * @brief Handles Compare: tab focus, progress, diff, colors, summary.
     ******************************************************************************/
    void actionCompare(); // Clears panel, shows progress bar, colorizes diff

    /******************************************************************************
     * @brief Shows the About dialog.
     ******************************************************************************/
    void actionAbout();

    /******************************************************************************
     * @brief Shows the Help dialog with usage tips.
     ******************************************************************************/
    void actionHelp();

    /******************************************************************************
     * @brief Adds selected text from New into Original.
     ******************************************************************************/
    void actionAddSelectedCode();

    /******************************************************************************
     * @brief Copies selection from New into Original at cursor.
     ******************************************************************************/
    void actionCopySelected();

    /******************************************************************************
     * @brief Moves selection from New into Original, removing from New.
     ******************************************************************************/
    void actionMoveSelected();

    /******************************************************************************
     * @brief Makes the Compare tab active.
     ******************************************************************************/
    void actionMakeCompareTabActive();

    /******************************************************************************
     * @brief Saves the Original editor content (or Save As).
     ******************************************************************************/
    void actionSaveOriginal();

    /******************************************************************************
     * @brief Saves the New editor content (or Save As).
     ******************************************************************************/
    void actionSaveNew();

    /******************************************************************************
     * @brief Save Original As to a chosen path, then save.
     ******************************************************************************/
    void actionSaveOriginalAs();

    /******************************************************************************
     * @brief Save New As to a chosen path, then save.
     ******************************************************************************/
    void actionSaveNewAs();

    /******************************************************************************
     * @brief Toggle overwrite warning setting.
     * @param on True to enable warning, false to disable.
     ******************************************************************************/
    void actionToggleOverwriteWarning(bool on);

    /******************************************************************************
     * @brief Tests the code in the Temporary path using CMake.
     ******************************************************************************/
    void actionCompile();

    /******************************************************************************
     * @brief Browse for CMakeLists.txt and save to settings.
     ******************************************************************************/
    void browseCMakePath();

    /******************************************************************************
     * @brief Browse for Temp folder and update view root.
     ******************************************************************************/
    void browseTempPath();

    /******************************************************************************
     * @brief Loads the CMake project into the temp folder, backing up any existing
     *        temp contents to a timestamped backup folder (excluding build).
     ******************************************************************************/
    void loadCMakeProject();

    /******************************************************************************
     * @brief Clear Temp folder contents recursively.
     ******************************************************************************/
    void clearTempFolder();

    /******************************************************************************
     * @brief Saves the contents of the temp file editor to the currently selected file.
     ******************************************************************************/
    void actionSaveTempFile();

    /******************************************************************************
     * @brief Copies the contents of the temp file editor to the clipboard.
     ******************************************************************************/
    void actionCopyTempFile();

    /******************************************************************************
     * @brief Pastes clipboard contents into the temp file editor.
     ******************************************************************************/
    void actionPasteTempFile();

    /******************************************************************************
     * @brief Adds a new project to the projects list.
     ******************************************************************************/
    void actionAddProject();

    /******************************************************************************
     * @brief Deletes the selected project from the projects list.
     ******************************************************************************/
    void actionDeleteProject();

    /******************************************************************************
     * @brief Loads the selected project (placeholder for future logic).
     ******************************************************************************/
    void actionLoadProject();

    /******************************************************************************
     * @brief Slot for Save button in Settings tab.
     *        Validates paths and saves to QSettings.
     ******************************************************************************/
    void saveSettings();

private:
    /******************************************************************************
     * @brief Sets up the main window UI, including all tabs and panels.
     ******************************************************************************/
    void setupUi();

    /******************************************************************************
     * @brief Connects actions and widgets to their slots.
     ******************************************************************************/
    void wireActions();

    /******************************************************************************
     * @brief Enables Add/Copy/Move depending on New selection.
     ******************************************************************************/
    void updateAddSelectedEnabled();

    /**************************************************************************
     * @brief Colorizes diff output in the comparison panel.
     *        - Red for removed (-)
     *        - Green for new (+)
     *        - Magenta for reordered (~)
     *        - Black for unchanged ( )
     * @param diff Diff result from CompareEngine.
     *************************************************************************/
    void setPaneColorsFromDiff(const QVector<QVector<QString>> &diff,
                               const QString &fileName, const QString &functionName);

    /******************************************************************************
     * @brief Appends readable diff text per function into comparison.
     * @param functionName Name of the function.
     * @param diff Diff result from CompareEngine.
     ******************************************************************************/
    void appendComparisonText(const QString &functionName,
                              const QVector<QVector<QString>> &diff);

    /******************************************************************************
     * @brief Warns before overwriting an existing file, with YesToAll.
     * @param path Path to the file.
     * @param onConfirm Callback to execute if confirmed.
     ******************************************************************************/
    void warnOverwriteIfNeeded(const QString &path, std::function<void()> onConfirm);

    /******************************************************************************
     * @brief Writes UTF-8 text to file, creating parent folder.
     * @param path Path to the file.
     * @param text Text to write.
     ******************************************************************************/
    void saveTextToFile(const QString &path, const QString &text);

    /******************************************************************************
     * @brief Ensure Temp root folder exists.
     ******************************************************************************/
    void ensureTempRoot();

    /******************************************************************************
     * @brief Copy CMakeLists and sibling sources into Temp (heuristic).
     * @param cmakePath Path to CMakeLists.txt.
     * @param tempRoot Path to temp folder.
     ******************************************************************************/
    void importCMakeToTemp(const QString &cmakePath, const QString &tempRoot);

    /******************************************************************************
     * @brief Minimal CMake scanning placeholder; returns matches.
     * @param cmakePath Path to CMakeLists.txt.
     * @return Vector of file paths.
     ******************************************************************************/
    QVector<QString> gatherProjectFilesFromCMake(const QString &cmakePath);

    /******************************************************************************
     * @brief Extracts the project name from a CMakeLists.txt file.
     * @param cmakePath Absolute path to CMakeLists.txt.
     * @return Project name, or "CodeHelpAI" if not found.
     ******************************************************************************/
    QString extractProjectNameFromCMake(const QString &cmakePath) const;

    /******************************************************************************
     * @brief Returns the temp folder path, one level up from project root,
     *        named after the project (e.g., PROJECTNAME_Temp).
     * @return Absolute path to temp folder.
     ******************************************************************************/
    QString currentTempRoot() const;

    /******************************************************************************
     * @brief Returns the backup folder path, one level up from project root,
     *        named after the project (e.g., PROJECTNAME_Backups).
     * @return Absolute path to backup folder.
     ******************************************************************************/
    QString currentBackupRoot() const;

    /******************************************************************************
     * @brief Opens a dialog to select the backup folder and updates settings.
     ******************************************************************************/
    void browseBackupPath();

    /******************************************************************************
     * @brief Recursively copies files and folders from srcPath to dstPath.
     * @param srcPath Source path.
     * @param dstPath Destination path.
     * @return true if successful, false otherwise.
     ******************************************************************************/
    bool copyRecursively(const QString &srcPath, const QString &dstPath);

    /******************************************************************************
     * @brief Extracts source/header file paths from CMakeLists.txt.
     * @param cmakeFilePath Path to CMakeLists.txt.
     * @return List of relative file paths to check.
     ******************************************************************************/
    QStringList extractCMakeSourceFiles(const QString &cmakeFilePath);

    // ==== Private members ====
    QTabWidget *tabs;                  ///< Main tab widget for all panels.
    QWidget *tabCompare;               ///< Compare tab for code comparison.
    QWidget *tabSettings;              ///< Settings tab for paths and options.
    QWidget *tabTemp;                  ///< Temp tab for temporary file operations.
    QWidget *tabProjects;              ///< Projects tab for project management.

    QSplitter *compareTopSplitter;     ///< Splitter for top section of Compare tab.
    QTextEdit *originalEdit;           ///< Editor for original code.
    QTextEdit *newEdit;                ///< Editor for new code.
    QPlainTextEdit *comparisonEdit;    ///< Output panel for comparison results.
    QPushButton *compareButton;        ///< Button to trigger code comparison.

    QLineEdit *cmakePathEdit;          ///< Edit field for CMakeLists.txt path.
    QPushButton *cmakeBrowseButton;    ///< Button to browse for CMakeLists.txt.
    QPushButton *cmakeLoadButton;      ///< Button to load CMake project.

    QLineEdit *tempPathEdit;           ///< Edit field for temporary folder path.
    QPushButton *tempBrowseButton;     ///< Button to browse for temp folder.

    QLineEdit *backupPathEdit;         ///< Edit field for backup folder path.
    QPushButton *saveSettingsButton;   ///< Button to save settings in Settings tab.

    QSplitter *tempSplitter;           ///< Splitter for Temp tab panels.
    QTreeView *tempTree;               ///< Tree view for temp folder files.
    QFileSystemModel *tempModel;       ///< Model for temp folder file system.
    QTextEdit *tempFileEdit;           ///< Editor for temp file content.

    QPushButton *tempSaveButton;       ///< Button to save temp file.
    QPushButton *tempCopyButton;       ///< Button to copy temp file content.
    QPushButton *tempPasteButton;      ///< Button to paste into temp file editor.
    QPushButton *tempClearButton;      ///< Button to clear temp folder.

    QListWidget *projectsList;         ///< List widget for projects.
    QPushButton *projectsAddButton;    ///< Button to add a new project.
    QPushButton *projectsDeleteButton; ///< Button to delete selected project.
    QPushButton *projectsLoadButton;   ///< Button to load selected project.
    QLineEdit *projectNameEdit;        ///< Edit field for project name.

    QAction *actOpenOriginal;          ///< Action to open original file.
    QAction *actOpenNew;               ///< Action to open new file.
    QAction *actSaveComparison;        ///< Action to save comparison output.
    QAction *actCompare;               ///< Action to trigger comparison.
    QAction *actAbout;                 ///< Action to show About dialog.
    QAction *actHelp;                  ///< Action to show Help dialog.
    QAction *actAddSelected;           ///< Action to add selected code.
    QAction *actCopySelected;          ///< Action to copy selected code.
    QAction *actMoveSelected;          ///< Action to move selected code.
    QAction *actMakeCompareActive;     ///< Action to activate Compare tab.
    QAction *actSaveOriginal;          ///< Action to save original file.
    QAction *actSaveNew;               ///< Action to save new file.
    QAction *actSaveOriginalAs;        ///< Action to save original file as.
    QAction *actSaveNewAs;             ///< Action to save new file as.
    QAction *actCompile;               ///< Action to compile code.
    QAction *actOverwriteWarn;         ///< Action to toggle overwrite warning.

    QSettings settings;                ///< Persistent settings storage.
    bool overwriteWarn;                ///< Flag for overwrite warning.
    QString originalPath;              ///< Path to original file.
    QString newPath;                   ///< Path to new file.
    CompareEngine *compareEngine;      ///< Function-aware code comparison engine.

    QPushButton *backupBrowseButton;   ///< Button to browse for backup folder.
    QPushButton *openButton;           ///< Button to open original file.
    QPushButton *openNewButton;        ///< Button to open new file.

};

/*************** End of MainWindow.h ***************************************/
