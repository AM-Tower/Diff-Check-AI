/******************************************************************************
 * @file MainWindow.cpp
 * @brief Implements the main application window for DiffCheckAI.
 *
 * @author Jeffrey Scott Flesher with the help of AI: Copilot
 * @version 0.8
 * @date 2025-11-07
 * @section License MIT
 * @section DESCRIPTION
 * Dynamic UI, function-aware compare, colorization:
 * - Green: same (style-insensitive)
 * - Red: deleted from original
 * - Yellow: added in new
 * - Cyan: reordered
 * Editable panes with undo/redo, save/open, overwrite warning.
 * Settings tab for CMake path, Temp path, Backup path.
 * Basic Temp/Projects scaffolding with file viewing/editing.
 ******************************************************************************/

#include "MainWindow.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QRegularExpression>
#include <QSplitter>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QToolBar>
#include <QVBoxLayout>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QColor>
#include <QTimer>
#include <tuple>

/******************************************************************************
 * @brief Constructor.
 * @param parent Parent widget.
 ******************************************************************************/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    tabs(nullptr),
    tabCompare(nullptr),
    tabSettings(nullptr),
    tabTemp(nullptr),
    tabProjects(nullptr),
    compareTopSplitter(nullptr),
    originalEdit(nullptr),
    newEdit(nullptr),
    comparisonEdit(nullptr),
    compareButton(nullptr),
    cmakePathEdit(nullptr),
    cmakeBrowseButton(nullptr),
    cmakeLoadButton(nullptr),
    tempPathEdit(nullptr),
    tempBrowseButton(nullptr),
    backupPathEdit(nullptr),
    tempSplitter(nullptr),
    tempTree(nullptr),
    tempModel(nullptr),
    tempFileEdit(nullptr),
    tempSaveButton(nullptr),
    tempCopyButton(nullptr),
    tempPasteButton(nullptr),
    tempClearButton(nullptr),
    projectsList(nullptr),
    projectsAddButton(nullptr),
    projectsDeleteButton(nullptr),
    projectsLoadButton(nullptr),
    projectNameEdit(nullptr),
    actOpenOriginal(nullptr),
    actOpenNew(nullptr),
    actSaveComparison(nullptr),
    actCompare(nullptr),
    actAbout(nullptr),
    actHelp(nullptr),
    actAddSelected(nullptr),
    actCopySelected(nullptr),
    actMoveSelected(nullptr),
    actMakeCompareActive(nullptr),
    actSaveOriginal(nullptr),
    actSaveNew(nullptr),
    actSaveOriginalAs(nullptr),
    actSaveNewAs(nullptr),
    actCompile(nullptr),
    actOverwriteWarn(nullptr),
    overwriteWarn(true),
    originalPath(),
    newPath(),
    compareEngine(new CompareEngine())
{
    // QtSettings
    appSettings = new Settings(QDir::currentPath() + "/data/settings.json");
    appSettings->load();

    setupUi();
    wireActions();
    updateAddSelectedEnabled();
    // Colorized Status bar
    colorLabel = nullptr;
    statusQueueStop = false;
    statusQueueThread = std::thread(&MainWindow::statusQueueWorker, this);
    queueStatusMessage(tr("Ready."), 6000);
}

/******************************************************************************
 * @brief Destructor.
 ******************************************************************************/
MainWindow::~MainWindow()
{
    delete compareEngine;
    delete appSettings;
    // Status bar queue
    {
        std::lock_guard<std::mutex> lock(statusQueueMutex);
        statusQueueStop = true;
    }
    statusQueueCV.notify_all();
    if (statusQueueThread.joinable())
    {
        statusQueueThread.join();
    }
}

/******************************************************************************
 * @brief Sets up the main window UI, including all tabs and panels.
 ******************************************************************************/
void MainWindow::setupUi()
{
    setWindowTitle(tr("DiffCheckAI"));
    resize(1100, 700);

    // Menu bar and tool bar
    QMenuBar *mb = new QMenuBar(this);
    setMenuBar(mb);
    QToolBar *tb = new QToolBar(tr("Main Toolbar"), this);
    addToolBar(Qt::TopToolBarArea, tb);
    QStatusBar *sb = new QStatusBar(this);
    setStatusBar(sb);

    // Actions
    actOpenOriginal = new QAction(QIcon(":/icons/icons/open.svg"), tr("Open Original..."), this);
    actOpenNew = new QAction(QIcon(":/icons/icons/open_new.svg"), tr("Open New..."), this);
    actSaveOriginal = new QAction(QIcon(":/icons/icons/save.svg"), tr("Save Original"), this);
    actSaveNew = new QAction(QIcon(":/icons/icons/save.svg"), tr("Save New"), this);
    actSaveOriginalAs = new QAction(QIcon(":/icons/icons/save.svg"), tr("Save Original As..."), this);
    actSaveNewAs = new QAction(QIcon(":/icons/icons/save.svg"), tr("Save New As..."), this);
    actSaveComparison = new QAction(QIcon(":/icons/icons/save.svg"), tr("Save Comparison..."), this);
    actCompare = new QAction(QIcon(":/icons/icons/compare.svg"), tr("Compare"), this);
    actAbout = new QAction(QIcon(":/icons/icons/about.svg"), tr("About"), this);
    actHelp = new QAction(QIcon(":/icons/icons/help.svg"), tr("Help"), this);
    actAddSelected = new QAction(tr("Add Selected Code"), this);
    actCopySelected = new QAction(tr("Copy Selected"), this);
    actMoveSelected = new QAction(tr("Move Selected"), this);
    actCompile = new QAction(QIcon(":/icons/icons/compile.svg"), tr("Compile"), this);
    actOverwriteWarn = new QAction(tr("Warn on Overwrite"), this);
    actOverwriteWarn->setCheckable(true);
    actOverwriteWarn->setChecked(overwriteWarn);

    // Menus
    QMenu *fileMenu = mb->addMenu(tr("&File"));
    fileMenu->addAction(actOpenOriginal);
    fileMenu->addAction(actOpenNew);
    fileMenu->addSeparator();
    fileMenu->addAction(actSaveOriginal);
    fileMenu->addAction(actSaveOriginalAs);
    fileMenu->addAction(actSaveNew);
    fileMenu->addAction(actSaveNewAs);
    fileMenu->addAction(actSaveComparison);
    fileMenu->addSeparator();
    fileMenu->addAction(actOverwriteWarn);

    QMenu *editMenu = mb->addMenu(tr("&Edit"));
    editMenu->addAction(actAddSelected);
    editMenu->addAction(actCopySelected);
    editMenu->addAction(actMoveSelected);

    QMenu *toolsMenu = mb->addMenu(tr("&Tools"));
    toolsMenu->addAction(actCompare);
    toolsMenu->addAction(actCompile);

    QMenu *helpMenu = mb->addMenu(tr("&Help"));
    helpMenu->addAction(actHelp);
    helpMenu->addAction(actAbout);

    // Toolbar
    tb->addAction(actOpenOriginal);
    tb->addAction(actOpenNew);
    tb->addAction(actCompare);
    tb->addAction(actSaveComparison);
    tb->addAction(actCompile);
    tb->addAction(actAbout);
    tb->addAction(actHelp);

    // Tabs
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    // --- Compare tab ---
    tabCompare = new QWidget(this);
    QSplitter *verticalSplitter = new QSplitter(Qt::Vertical, tabCompare);

    // Top panel: Open buttons and editors
    QWidget *topPanel = new QWidget(verticalSplitter);
    QVBoxLayout *topPanelLayout = new QVBoxLayout(topPanel);
    QHBoxLayout *openButtonsLayout = new QHBoxLayout();
    openButton = new QPushButton(tr("Open"), topPanel);
    openNewButton = new QPushButton(tr("Open New"), topPanel);
    openButtonsLayout->addWidget(openButton);
    openButtonsLayout->addWidget(openNewButton);

    QSplitter *editSplitter = new QSplitter(Qt::Horizontal, topPanel);
    originalEdit = new QTextEdit(editSplitter);
    newEdit = new QTextEdit(editSplitter);
    originalEdit->setPlaceholderText(tr("Original code (open or paste here)..."));
    newEdit->setPlaceholderText(tr("New code (open or paste here)..."));
    editSplitter->addWidget(originalEdit);
    editSplitter->addWidget(newEdit);
    editSplitter->setStretchFactor(0, 1);
    editSplitter->setStretchFactor(1, 1);

    topPanelLayout->addLayout(openButtonsLayout);
    topPanelLayout->addWidget(editSplitter);

    // Bottom panel: Compare button and output
    QWidget *bottomPanel = new QWidget(verticalSplitter);
    QVBoxLayout *bottomPanelLayout = new QVBoxLayout(bottomPanel);
    compareButton = new QPushButton(tr("Compare"), bottomPanel);
    comparisonEdit = new QPlainTextEdit(bottomPanel);
    comparisonEdit->setPlaceholderText(tr("Comparison output and summary..."));
    comparisonEdit->setReadOnly(false);
    bottomPanelLayout->addWidget(compareButton);
    bottomPanelLayout->addWidget(comparisonEdit);

    verticalSplitter->addWidget(topPanel);
    verticalSplitter->addWidget(bottomPanel);
    verticalSplitter->setStretchFactor(0, 1);
    verticalSplitter->setStretchFactor(1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout(tabCompare);
    mainLayout->addWidget(verticalSplitter);
    tabCompare->setLayout(mainLayout);
    tabs->addTab(tabCompare, tr("Compare"));

    // --- Settings tab ---
    tabSettings = new QWidget(this);
    QFormLayout *setForm = new QFormLayout(tabSettings);

    // CMake path row
    cmakePathEdit = new QLineEdit(tabSettings);
    cmakePathEdit->setObjectName("cmakePathEdit");
    cmakeBrowseButton = new QPushButton(tr("Browse..."), tabSettings);
    cmakeLoadButton = new QPushButton(tr("Load"), tabSettings);
    QWidget *cmakeRow = new QWidget(tabSettings);
    QHBoxLayout *cmakeRowLayout = new QHBoxLayout(cmakeRow);
    cmakeRowLayout->addWidget(cmakePathEdit);
    cmakeRowLayout->addWidget(cmakeBrowseButton);
    cmakeRowLayout->addWidget(cmakeLoadButton);
    setForm->addRow(tr("CMakeLists.txt:"), cmakeRow);

    // Temp path row
    tempPathEdit = new QLineEdit(tabSettings);
    tempPathEdit->setObjectName("tempPathEdit");
    tempBrowseButton = new QPushButton(tr("Browse..."), tabSettings);
    QWidget *tempRow = new QWidget(tabSettings);
    QHBoxLayout *tempRowLayout = new QHBoxLayout(tempRow);
    tempRowLayout->addWidget(tempPathEdit);
    tempRowLayout->addWidget(tempBrowseButton);
    setForm->addRow(tr("Temporary Path:"), tempRow);
    tempPathEdit->setEnabled(false);
    // Backup path row
    backupPathEdit = new QLineEdit(tabSettings);
    backupPathEdit->setObjectName("backupPathEdit");
    backupBrowseButton = new QPushButton(tr("Browse..."), tabSettings);
    QWidget *backupRow = new QWidget(tabSettings);
    QHBoxLayout *backupRowLayout = new QHBoxLayout(backupRow);
    backupRowLayout->addWidget(backupPathEdit);
    backupRowLayout->addWidget(backupBrowseButton);
    setForm->addRow(tr("Backup Path:"), backupRow);
    backupPathEdit->setEnabled(false);

    // Save button for settings
    saveSettingsButton = new QPushButton(tr("Save"), tabSettings);
    saveSettingsButton->setObjectName("saveSettingsButton");
    setForm->addRow(saveSettingsButton);

    // Load settings
    cmakePathEdit->setText(appSettings->value("paths/cmake", "").toString());
    tempPathEdit->setText(appSettings->value("paths/temp", QDir::homePath() + "/DiffCheckAI_Temp").toString());
    backupPathEdit->setText(appSettings->value("paths/backup", QDir::homePath() + "/DiffCheckAI_Backups").toString());

    tabSettings->setLayout(setForm);
    tabs->addTab(tabSettings, tr("Settings"));

    // --- Temp tab ---
    tabTemp = new QWidget(this);
    QVBoxLayout *tempLayout = new QVBoxLayout(tabTemp);
    tempSplitter = new QSplitter(Qt::Horizontal, tabTemp);
    tempModel = new QFileSystemModel(tempSplitter);
    tempModel->setRootPath(currentTempRoot());
    tempTree = new QTreeView(tempSplitter);
    tempTree->setModel(tempModel);
    tempTree->setRootIndex(tempModel->index(currentTempRoot()));
    tempFileEdit = new QTextEdit(tempSplitter);
    tempFileEdit->setAcceptRichText(false);
    tempSplitter->addWidget(tempTree);
    tempSplitter->addWidget(tempFileEdit);

    QHBoxLayout *tempButtons = new QHBoxLayout();
    tempSaveButton = new QPushButton(tr("Save"), tabTemp);
    tempCopyButton = new QPushButton(tr("Copy"), tabTemp);
    tempPasteButton = new QPushButton(tr("Paste"), tabTemp);
    tempClearButton = new QPushButton(tr("Clear Temp Folder"), tabTemp);
    tempButtons->addWidget(tempSaveButton);
    tempButtons->addWidget(tempCopyButton);
    tempButtons->addWidget(tempPasteButton);
    tempButtons->addWidget(tempClearButton);

    tempLayout->addWidget(tempSplitter);
    tempLayout->addLayout(tempButtons);
    tabTemp->setLayout(tempLayout);
    tabs->addTab(tabTemp, tr("Temp"));

    // --- Projects tab ---
    tabProjects = new QWidget(this);
    QVBoxLayout *projLayout = new QVBoxLayout(tabProjects);
    projectNameEdit = new QLineEdit(tabProjects);
    projectNameEdit->setPlaceholderText(tr("Project Name"));
    projectsList = new QListWidget(tabProjects);
    projectsAddButton = new QPushButton(tr("Add"), tabProjects);
    projectsDeleteButton = new QPushButton(tr("Delete"), tabProjects);
    projectsLoadButton = new QPushButton(tr("Load"), tabProjects);
    QHBoxLayout *projBtnLayout = new QHBoxLayout();
    projBtnLayout->addWidget(projectsAddButton);
    projBtnLayout->addWidget(projectsDeleteButton);
    projBtnLayout->addWidget(projectsLoadButton);
    projLayout->addWidget(projectNameEdit);
    projLayout->addWidget(projectsList);
    projLayout->addLayout(projBtnLayout);
    tabProjects->setLayout(projLayout);
    tabs->addTab(tabProjects, tr("Projects"));
}

/******************************************************************************
 * @brief Connects actions and widgets to their slots.
 ******************************************************************************/
void MainWindow::wireActions()
{
    // Compare tab
    connect(openButton, &QPushButton::clicked, this, &MainWindow::actionOpenOriginal);
    connect(openNewButton, &QPushButton::clicked, this, &MainWindow::actionOpenNew);
    connect(compareButton, &QPushButton::clicked, this, &MainWindow::actionCompare);

    // Settings tab
    connect(cmakeBrowseButton, &QPushButton::clicked, this, &MainWindow::browseCMakePath);
    connect(tempBrowseButton, &QPushButton::clicked, this, &MainWindow::browseTempPath);
    connect(backupBrowseButton, &QPushButton::clicked, this, &MainWindow::browseBackupPath);
    connect(cmakeLoadButton, &QPushButton::clicked, this, &MainWindow::loadCMakeProject);

    // Save button for Settings tab
    connect(saveSettingsButton, &QPushButton::clicked, this, &MainWindow::saveSettings);

    // Temp tab
    connect(tempSaveButton, &QPushButton::clicked, [this]()
            {
                QModelIndex idx = tempTree->currentIndex();
                if (!idx.isValid()) { return; }
                QString path = tempModel->filePath(idx);
                warnOverwriteIfNeeded(path, [this, path]()
                                      {
                                          saveTextToFile(path, tempFileEdit->toPlainText());
                                      });
            });
    connect(tempCopyButton, &QPushButton::clicked, this, &MainWindow::actionCopyTempFile);
    connect(tempPasteButton, &QPushButton::clicked, this, &MainWindow::actionPasteTempFile);
    connect(tempClearButton, &QPushButton::clicked, this, &MainWindow::clearTempFolder);

    // Projects tab
    connect(projectsAddButton, &QPushButton::clicked, this, &MainWindow::actionAddProject);
    connect(projectsDeleteButton, &QPushButton::clicked, this, &MainWindow::actionDeleteProject);
    connect(projectsLoadButton, &QPushButton::clicked, this, &MainWindow::actionLoadProject);

    connect(actCompile, &QAction::triggered, this, &MainWindow::actionCompile);
    // Cmake path change
    connect(cmakePathEdit, &QLineEdit::textChanged, this, &MainWindow::onCmakePathChanged);
    // Other connects as needed (menus, toolbar, etc.)
    // ... (add any additional connects here, but ensure no duplicates) ...
}

/******************************************************************************
 * @brief Slot for Save button in Settings tab.
 *        Validates paths and saves to QSettings.
 ******************************************************************************/
bool MainWindow::saveSettings()
{
    QString cmakePath = cmakePathEdit->text().trimmed();
    QString tempPath = tempPathEdit->text().trimmed();
    QString backupPath = backupPathEdit->text().trimmed();

    // Validate CMake path
    QFileInfo cmakeFileInfo(cmakePath);
    if (!cmakeFileInfo.exists() || !cmakeFileInfo.isFile())
    {
        queueStatusMessage(tr("Invalid CMake Path: does not exist or is not a file."), 3000, Qt::red);
        return false;
    }

    // Validate temp and backup folders
    QDir tempDir(tempPath);
    QDir backupDir(backupPath);

    if (!tempDir.exists())
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Create Temp Folder"),
            tr("Temp folder does not exist:\n%1\nCreate it?").arg(tempPath),
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::Yes)
        {
            if (!QDir().mkpath(tempPath))
            {
                queueStatusMessage(tr("Failed to create Temp folder."), 3000, Qt::red);
                return false;
            }
        }
        else
        {
            queueStatusMessage(tr("Temp folder not created. Change name or path."), 3000, Qt::red);
            return false;
        }
    }

    if (!backupDir.exists())
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Create Backup Folder"),
            tr("Backup folder does not exist:\n%1\nCreate it?").arg(backupPath),
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::Yes)
        {
            if (!QDir().mkpath(backupPath))
            {
                queueStatusMessage(tr("Failed to create Backup folder."), 3000, Qt::red);
                return false;
            }
        }
        else
        {
            queueStatusMessage(tr("Backup folder not created. Change name or path."), 3000, Qt::red);
            return false;
        }
    }

    // Save settings
    appSettings->setValue("paths/cmake", cmakePath);
    appSettings->setValue("paths/temp", tempPath);
    appSettings->setValue("paths/backup", backupPath);
    appSettings->save();

    queueStatusMessage(tr("Settings Saved: All paths validated and saved."), 5000, Qt::green);
    return true;
}
/******************************************************************************
 * @brief Open a file into the Original editor.
 ******************************************************************************/
void MainWindow::actionOpenOriginal()
{
    QString dir = currentTempRoot();
    QString path = QFileDialog::getOpenFileName(this, tr("Open Original"), dir, tr("Code Files (*.h *.hpp *.c *.cpp *.cc *.txt);;All Files (*)"));
    if (path.isEmpty()) { return; }
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        originalEdit->setPlainText(QString::fromUtf8(f.readAll()));
        originalPath = path;
        queueStatusMessage(tr("Opened original: %1").arg(path));
    }
}

/******************************************************************************
 * @brief Opens a dialog to select the backup folder and updates settings.
 ******************************************************************************/
void MainWindow::browseBackupPath()
{
    // Open a folder selection dialog, starting at the current backup path
    QString currentPath = backupPathEdit->text();
    QString path = QFileDialog::getExistingDirectory(
        this,
        tr("Select Backup Folder"),
        currentPath.isEmpty() ? QDir::homePath() : currentPath
        );

    // If the user selected a folder, update the UI and settings
    if (!path.isEmpty())
    {
        backupPathEdit->setText(path);
        appSettings->setValue("paths/backup", path);
        appSettings->save();

        queueStatusMessage(tr("Backup folder set to: %1").arg(path));
    }
}

/******************************************************************************
 * @brief Open a file into the New editor.
 ******************************************************************************/
void MainWindow::actionOpenNew()
{
    QString dir = currentTempRoot();
    QString path = QFileDialog::getOpenFileName(this, tr("Open New"), dir, tr("Code Files (*.h *.hpp *.c *.cpp *.cc *.txt);;All Files (*)"));
    if (path.isEmpty()) { return; }
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        newEdit->setPlainText(QString::fromUtf8(f.readAll()));
        newPath = path;
        queueStatusMessage(tr("Opened new: %1").arg(path), 2000);
    }
}

/******************************************************************************
 * @brief Save comparison output to a file with overwrite warn.
 ******************************************************************************/
void MainWindow::actionSaveComparison()
{
    QString dir = currentTempRoot();
    QString path = QFileDialog::getSaveFileName(this, tr("Save Comparison"), dir, tr("Text Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) { return; }
    warnOverwriteIfNeeded(path, [this, path]() {
        saveTextToFile(path, comparisonEdit->toPlainText());
        queueStatusMessage(tr("Saved comparison: %1").arg(path), 2000);
    });
}

/******************************************************************************
 * @brief Handles Compare: tab focus, progress, diff, colors, summary.
 ******************************************************************************/
void MainWindow::actionCompare()
{
    // Switch to Compare tab
    tabs->setCurrentWidget(tabCompare);

    // Clear comparison panel
    comparisonEdit->clear();

    // Get the code from the editors
    QString orig = originalEdit->toPlainText();
    QString news = newEdit->toPlainText();

    // Extract functions before using origMap and newMap
    auto origMap = compareEngine->extractFunctions(orig);
    auto newMap = compareEngine->extractFunctions(news);

    // Get file name from originalPath (or fallback)
    QString fileName = originalPath.isEmpty() ? tr("Original") : QFileInfo(originalPath).fileName();

    // Show progress bar in status bar
    int totalSteps = origMap.size();
    QProgressBar *progress = new QProgressBar(this);
    progress->setRange(0, totalSteps);
    statusBar()->addPermanentWidget(progress);
    queueStatusMessage(tr("Comparison started..."));

    int step = 0;
    for (auto it = origMap.constBegin(); it != origMap.constEnd(); ++it)
    {
        const QString &fname = it.key();
        if (!newMap.contains(fname)) { continue; }
        const auto &fo = it.value();
        const auto &fn = newMap.value(fname);
        auto diff = compareEngine->diffLines(fo.bodyLines, fn.bodyLines);

        // Pass all required arguments: diff, fileName, functionName
        setPaneColorsFromDiff(diff, fileName, fname);

        progress->setValue(++step);
        QApplication::processEvents();
    }

    // Optionally colorize summary as well
    QTextCursor cursor(comparisonEdit->document());
    QTextCharFormat fmt;
    fmt.setForeground(Qt::darkGray);
    cursor.insertText("\n" + compareEngine->buildSummary(origMap, newMap) + "\n", fmt);

    statusBar()->removeWidget(progress);
    progress->deleteLater();
    queueStatusMessage(tr("Comparison finished"), 3000);
}

/******************************************************************************
 * @brief Color-codes the lower comparison panel from diff results.
 ******************************************************************************/
void MainWindow::setPaneColorsFromDiff(const QVector<QVector<QString>> &diff,
                                       const QString &fileName,
                                       const QString &functionName)
{
    QTextCursor cursor(comparisonEdit->document());

    // Divider
    QTextCharFormat dividerFmt;
    dividerFmt.setForeground(Qt::darkGray);
    cursor.insertText("-----------------------------------------------------------------\n", dividerFmt);

    // File and function header
    QTextCharFormat headerFmt;
    headerFmt.setForeground(Qt::blue);
    cursor.insertText(QString("File: %1\nFunction: %2\n\n").arg(fileName, functionName), headerFmt);

    // Diff lines
    for (const auto &triple : diff)
    {
        QTextCharFormat fmt;
        if (triple[0] == "+")
            fmt.setForeground(Qt::green);
        else if (triple[0] == "-")
            fmt.setForeground(Qt::red);
        else if (triple[0] == "~")
            fmt.setForeground(Qt::magenta);
        else
            fmt.setForeground(Qt::black);

        QString line;
        if (triple[0] == "+")
            line = "+ " + triple[2];
        else if (triple[0] == "-")
            line = "- " + triple[1];
        else if (triple[0] == "~")
            line = "~ " + (triple[2].isEmpty() ? triple[1] : triple[2]);
        else
            line = " " + triple[1];

        cursor.insertText(line + "\n", fmt);
    }
    cursor.insertText("\n", dividerFmt); // Optional: another divider after each diff
}

/******************************************************************************
 * @brief Appends readable diff text per function into comparison.
 ******************************************************************************/
void MainWindow::appendComparisonText(const QString &functionName,
                                      const QVector<QVector<QString>> &diff)
{
    comparisonEdit->appendPlainText(QString("=== Function: %1 ===").arg(functionName));
    for (const auto &row : diff)
    {
        const QString &marker = row[0];
        const QString &ol = row[1];
        const QString &nl = row[2];
        if (marker == " ")
        {
            comparisonEdit->appendPlainText(" " + ol);
        }
        else if (marker == "-")
        {
            comparisonEdit->appendPlainText("- " + ol);
        }
        else if (marker == "+")
        {
            comparisonEdit->appendPlainText("+ " + nl);
        }
        else if (marker == "~")
        {
            comparisonEdit->appendPlainText("~ " + (nl.isEmpty() ? ol : nl));
        }
    }
    comparisonEdit->appendPlainText(QString());
}

/******************************************************************************
 * @brief About dialog.
 ******************************************************************************/
void MainWindow::actionAbout()
{
    QMessageBox::about(this,
                       tr("About DiffCheckAI"),
                       tr("<b>DiffCheckAI</b><br>"
                          "Function-level code comparison with style-insensitive matching.<br>"
                          "Green = same, Red = deleted in original, Yellow = new, Cyan = reordered."));
}

/******************************************************************************
 * @brief Help dialog with usage tips.
 ******************************************************************************/
void MainWindow::actionHelp()
{
    QMessageBox::information(this,
                             tr("Help"),
                             tr("Open Original and New code files (or paste).\n"
                                "Press Compare to analyze functions with the same names.\n"
                                "Colors show differences; comparison pane shows a diff and summary.\n"
                                "Use Add/Copy/Move Selected to insert code from New into Original."));
}

/******************************************************************************
 * @brief Adds selected text from New into Original.
 ******************************************************************************/
void MainWindow::actionAddSelectedCode()
{
    QTextCursor newSel = newEdit->textCursor();
    if (!newSel.hasSelection()) { return; }
    QTextCursor origCur = originalEdit->textCursor();
    if (origCur.hasSelection())
    {
        origCur.insertText(newSel.selectedText());
    }
    else
    {
        origCur.movePosition(QTextCursor::StartOfLine);
        origCur.insertText(newSel.selectedText());
    }
}

/******************************************************************************
 * @brief Copies selection from New into Original at cursor.
 ******************************************************************************/
void MainWindow::actionCopySelected()
{
    QTextCursor newSel = newEdit->textCursor();
    if (!newSel.hasSelection()) { return; }
    originalEdit->textCursor().insertText(newSel.selectedText());
}

/******************************************************************************
 * @brief Moves selection from New into Original, removing from New.
 ******************************************************************************/
void MainWindow::actionMoveSelected()
{
    QTextCursor newSel = newEdit->textCursor();
    if (!newSel.hasSelection()) { return; }
    QString text = newSel.selectedText();
    newSel.removeSelectedText();
    QTextCursor origCur = originalEdit->textCursor();
    if (origCur.hasSelection())
    {
        origCur.insertText(text);
    }
    else
    {
        origCur.movePosition(QTextCursor::StartOfLine);
        origCur.insertText(text);
    }
}

/******************************************************************************
 * @brief Makes the Compare tab active.
 ******************************************************************************/
void MainWindow::actionMakeCompareTabActive()
{
    tabs->setCurrentWidget(tabCompare);
}

/******************************************************************************
 * @brief Enables Add/Copy/Move depending on New selection.
 ******************************************************************************/
void MainWindow::updateAddSelectedEnabled()
{
    bool hasSelectionNew = newEdit->textCursor().hasSelection();
    bool hasOrigLine = originalEdit->textCursor().block().isValid();
    actAddSelected->setEnabled(hasSelectionNew && hasOrigLine);
    actCopySelected->setEnabled(hasSelectionNew);
    actMoveSelected->setEnabled(hasSelectionNew);
}

/******************************************************************************
 * @brief Warns before overwriting an existing file, with YesToAll.
 ******************************************************************************/
void MainWindow::warnOverwriteIfNeeded(const QString &path, std::function<void()> onConfirm)
{
    if (!overwriteWarn || !QFileInfo::exists(path))
    {
        onConfirm();
        return;
    }
    auto ret = QMessageBox::warning(this,
                                    tr("Overwrite Warning"),
                                    tr("File exists:\n%1\nOverwrite?").arg(path),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll);
    if (ret == QMessageBox::Yes)
    {
        onConfirm();
    }
    else if (ret == QMessageBox::YesToAll)
    {
        overwriteWarn = false;
        appSettings->setValue("overwriteWarn", false);
        appSettings->save();

        onConfirm();
    }
}

/******************************************************************************
 * @brief Writes UTF-8 text to file, creating parent folder.
 ******************************************************************************/
void MainWindow::saveTextToFile(const QString &path, const QString &text)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Save Failed"), tr("Cannot write: %1").arg(path));
        return;
    }
    f.write(text.toUtf8());
    f.close();
}

/******************************************************************************
 * @brief Save the Original editor content (or Save As).
 ******************************************************************************/
void MainWindow::actionSaveOriginal()
{
    if (originalPath.isEmpty())
    {
        actionSaveOriginalAs();
        return;
    }
    warnOverwriteIfNeeded(originalPath, [this]() {
        saveTextToFile(originalPath, originalEdit->toPlainText());
        queueStatusMessage(tr("Saved original: %1").arg(originalPath), 2000);
    });
}

/******************************************************************************
 * @brief Save the New editor content (or Save As).
 ******************************************************************************/
void MainWindow::actionSaveNew()
{
    if (newPath.isEmpty())
    {
        actionSaveNewAs();
        return;
    }
    warnOverwriteIfNeeded(newPath, [this]() {
        saveTextToFile(newPath, newEdit->toPlainText());
        queueStatusMessage(tr("Saved new: %1").arg(newPath), 2000);
    });
}

/******************************************************************************
 * @brief Save Original As to a chosen path, then save.
 ******************************************************************************/
void MainWindow::actionSaveOriginalAs()
{
    QString dir = currentTempRoot();
    QString path = QFileDialog::getSaveFileName(this, tr("Save Original As"), dir, tr("Code Files (*.h *.hpp *.c *.cpp *.cc *.txt);;All Files (*)"));
    if (path.isEmpty()) { return; }
    originalPath = path;
    actionSaveOriginal();
}

/******************************************************************************
 * @brief Save New As to a chosen path, then save.
 ******************************************************************************/
void MainWindow::actionSaveNewAs()
{
    QString dir = currentTempRoot();
    QString path = QFileDialog::getSaveFileName(this, tr("Save New As"), dir, tr("Code Files (*.h *.hpp *.c *.cpp *.cc *.txt);;All Files (*)"));
    if (path.isEmpty()) { return; }
    newPath = path;
    actionSaveNew();
}

/******************************************************************************
 * @brief Toggle overwrite warning setting.
 ******************************************************************************/
void MainWindow::actionToggleOverwriteWarning(bool on)
{
    overwriteWarn = on;
    appSettings->setValue("overwriteWarn", overwriteWarn);
    appSettings->save();
}

/******************************************************************************
 * @brief Tests the code in the Temporary path using CMake.
 *        Checks for missing files as listed in CMakeLists.txt, warns if any are missing,
 *        and offers to run anyway. Runs CMake, shows output in Compare panel,
 *        pops up dialog with summary. Shows progress and scrolls to bottom.
 ******************************************************************************/
/******************************************************************************
 * @brief Tests the code in the Temporary path using CMake.
 *        Shows a progress bar during the operation.
 ******************************************************************************/
void MainWindow::actionCompile()
{
    QElapsedTimer timer;
    timer.start();

    // Switch to Compare tab and show progress immediately
    tabs->setCurrentWidget(tabCompare);
    comparisonEdit->clear();
    queueStatusMessage(tr("Compile started..."), 2000);

    // Create and show progress bar
    QProgressBar *progress = new QProgressBar(this);
    progress->setRange(0, 100);
    statusBar()->addPermanentWidget(progress);
    progress->setValue(0);
    QApplication::processEvents();

    // Step 1: Check paths
    QString tempRoot = tempPathEdit->text().trimmed();
    if (tempRoot.isEmpty())
    {
        statusBar()->removeWidget(progress);
        progress->deleteLater();
        QMessageBox::warning(this, tr("Missing Temporary Path"),
                             tr("Set the Temporary path in Settings."));
        return;
    }
    progress->setValue(10);
    QApplication::processEvents();

    QString cmakeFile = tempRoot + "/CMakeLists.txt";
    QFileInfo cmakeInfo(cmakeFile);
    if (!cmakeInfo.exists())
    {
        statusBar()->removeWidget(progress);
        progress->deleteLater();
        QMessageBox::critical(this, tr("Missing CMakeLists.txt"),
                              tr("CMakeLists.txt not found in:\n%1\nPlease load the project again.").arg(tempRoot));
        return;
    }
    progress->setValue(20);
    QApplication::processEvents();

    // Step 2: Check for missing source files
    QStringList sourceFiles = extractCMakeSourceFiles(cmakeFile);
    QStringList missingFiles;
    for (int i = 0; i < sourceFiles.size(); ++i)
    {
        const QString &relPath = sourceFiles.at(i);
        QString absPath = tempRoot + "/" + relPath;
        if (!QFileInfo::exists(absPath))
        {
            missingFiles << relPath;
        }
    }
    progress->setValue(30);
    QApplication::processEvents();

    if (!missingFiles.isEmpty())
    {
        statusBar()->removeWidget(progress);
        progress->deleteLater();
        QString msg = tr("The following required files are missing:\n%1\n\nRun CMake anyway?")
                          .arg(missingFiles.join("\n"));
        int ret = QMessageBox::warning(this, tr("Missing Files"), msg,
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes)
        {
            queueStatusMessage(tr("Compile cancelled."), 2000);
            return;
        }
        // Show progress again if user continues
        statusBar()->addPermanentWidget(progress);
        QApplication::processEvents();
    }
    progress->setValue(40);
    QApplication::processEvents();

    // Step 3: Prepare build directory
    QString buildDir = tempRoot + "/build";
    QDir buildDirObj(buildDir);
    if (buildDirObj.exists())
    {
        buildDirObj.removeRecursively();
    }
    QDir().mkpath(buildDir);
    comparisonEdit->appendPlainText("=== CMake Debug Info ===");
    comparisonEdit->appendPlainText(QString("Temp project folder: %1").arg(tempRoot));
    comparisonEdit->appendPlainText(QString("Build folder: %1").arg(buildDir));
    comparisonEdit->appendPlainText("========================\n");
    progress->setValue(50);
    QApplication::processEvents();

    // Step 4: Configure CMake
    QString generator;
#ifdef _MSC_VER
    generator = "Visual Studio 17 2022"; // Or match your installed MSVC version
#else
#ifdef __MINGW32__
    generator = "MinGW Makefiles";
#else
    generator = "Ninja"; // Fallback for other environments
#endif
#endif
    QProcess cmakeConfig;
    QStringList configArgs;
    configArgs << "-G" << generator << "-S" << tempRoot << "-B" << buildDir;
    cmakeConfig.setWorkingDirectory(tempRoot);
    cmakeConfig.start("cmake", configArgs);
    progress->setValue(60);
    QApplication::processEvents();

    if (!cmakeConfig.waitForStarted(5000))
    {
        statusBar()->removeWidget(progress);
        progress->deleteLater();
        QMessageBox::critical(this, tr("Build Error"),
                              tr("Failed to start CMake configuration process."));
        return;
    }
    cmakeConfig.waitForFinished(-1);
    QString configOut = QString::fromUtf8(cmakeConfig.readAllStandardOutput());
    QString configErr = QString::fromUtf8(cmakeConfig.readAllStandardError());
    comparisonEdit->appendPlainText("=== CMake Configure Output ===");
    comparisonEdit->appendPlainText(configOut);
    comparisonEdit->appendPlainText(configErr);
    progress->setValue(70);
    QApplication::processEvents();

    // Step 5: Build
    QProcess cmakeBuild;
    QStringList buildArgs;
    buildArgs << "--build" << buildDir;
    cmakeBuild.setWorkingDirectory(tempRoot);
    cmakeBuild.start("cmake", buildArgs);
    progress->setValue(80);
    QApplication::processEvents();

    if (!cmakeBuild.waitForStarted(5000))
    {
        statusBar()->removeWidget(progress);
        progress->deleteLater();
        QMessageBox::critical(this, tr("Build Error"),
                              tr("Failed to start CMake build process."));
        return;
    }
    cmakeBuild.waitForFinished(-1);
    QString buildOut = QString::fromUtf8(cmakeBuild.readAllStandardOutput());
    QString buildErr = QString::fromUtf8(cmakeBuild.readAllStandardError());
    comparisonEdit->appendPlainText("=== CMake Build Output ===");
    comparisonEdit->appendPlainText(buildOut);
    comparisonEdit->appendPlainText(buildErr);
    progress->setValue(90);
    QApplication::processEvents();

    // Step 6: Show summary and finish
    QTextCursor cursor(comparisonEdit->document());
    cursor.movePosition(QTextCursor::End);
    comparisonEdit->setTextCursor(cursor);
    progress->setValue(100);
    QApplication::processEvents();
    statusBar()->removeWidget(progress);
    progress->deleteLater();

    int errorCount = 0;
    int warningCount = 0;
    QString allOutput = configOut + "\n" + configErr + "\n" + buildOut + "\n" + buildErr;
    QStringList lines = allOutput.split('\n');
    for (int i = 0; i < lines.size(); ++i)
    {
        QString line = lines.at(i).toLower();
        if (line.contains("error"))
        {
            ++errorCount;
        }
        if (line.contains("warning"))
        {
            ++warningCount;
        }
    }
    QString summary = tr("Compile complete.\n\nErrors: %1\nWarnings: %2\n\nSee Compare panel for details.")
                          .arg(errorCount)
                          .arg(warningCount);

    // Calculate and show elapsed time
    qint64 ms = timer.elapsed();
    double seconds = ms / 1000.0;
    QString timeMsg = tr("Elapsed time: %1 seconds").arg(QString::number(seconds, 'f', 2));
    QMessageBox::information(this, tr("Compile Results"), summary + "\n\n" + timeMsg);
    queueStatusMessage(tr("Compile complete. ") + timeMsg, 5000);
    comparisonEdit->appendPlainText(timeMsg);

}

/******************************************************************************
 * @brief Parses CMakeLists.txt to extract source/header file paths from
 *        qt_add_executable or add_executable, ignoring resource/translation variables.
 *        Handles multiple items on one line. Uses index-based loops for Clazy.
 * @param cmakeFilePath Absolute path to CMakeLists.txt.
 * @return List of relative file paths to check.
 ******************************************************************************/
QStringList MainWindow::extractCMakeSourceFiles(const QString &cmakeFilePath)
{
    QStringList result;
    QFile file(cmakeFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return result;
    }
    QString contents = QString::fromUtf8(file.readAll());
    QRegularExpression exeRe(R"(qt_add_executable\s*\([^\n]*\n([^\)]*)\))", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch exeMatch = exeRe.match(contents);
    if (!exeMatch.hasMatch())
    {
        // Try add_executable if qt_add_executable not found
        exeRe.setPattern(R"(add_executable\s*\([^\n]*\n([^\)]*)\))");
        exeMatch = exeRe.match(contents);
    }
    if (exeMatch.hasMatch())
    {
        QString block = exeMatch.captured(1);
        QStringList lines = block.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
        for (int i = 0; i < lines.size(); ++i)
        {
            QString trimmed = lines.at(i).trimmed();
            // Ignore empty lines and comments
            if (trimmed.isEmpty() || trimmed.startsWith("#"))
            {
                continue;
            }
            // Split line by spaces to get individual file paths
            QStringList items = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            for (int j = 0; j < items.size(); ++j)
            {
                const QString &item = items.at(j);
                // Ignore variables and resource/translation files
                if (item.startsWith("${") || item.endsWith(".qrc") || item.endsWith(".qm"))
                {
                    continue;
                }
                result << item;
            }
        }
    }
    return result;
}

/******************************************************************************
 * @brief Browse for CMakeLists.txt and save to settings.
 ******************************************************************************/
void MainWindow::browseCMakePath()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select CMakeLists.txt"), QString(), tr("CMakeLists (CMakeLists.txt)"));
    if (path.isEmpty()) { return; }
    cmakePathEdit->setText(path);
    appSettings->setValue("paths/cmake", path);
    appSettings->save();
}

/******************************************************************************
 * @brief Browse for Temp folder and update view root.
 ******************************************************************************/
void MainWindow::browseTempPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Temp Folder"), tempPathEdit->text());
    if (path.isEmpty()) { return; }
    tempPathEdit->setText(path);
    appSettings->setValue("paths/temp", path);
    appSettings->save();
    tempModel->setRootPath(path);
    tempTree->setRootIndex(tempModel->index(path));
}

/******************************************************************************
 * @brief Loads the CMake project into the temp folder, backing up any existing
 *        temp contents to a timestamped backup folder (excluding build).
 *        Warns before deleting temp contents, then copies all files/folders
 *        from the CMake project (excluding build) into temp.
 *        Shows progress feedback for both backup and copy operations.
 ******************************************************************************/
void MainWindow::loadCMakeProject()
{
    QString cmakePath = cmakePathEdit->text().trimmed();
    QString tempRoot = tempPathEdit->text().trimmed();
    QString backupRoot = backupPathEdit->text().trimmed();

    if (cmakePath.isEmpty() || tempRoot.isEmpty() || backupRoot.isEmpty())
    {
        QMessageBox::warning(this, tr("Missing Path"),
                             tr("Set CMakeLists.txt, Temp folder, and Backup folder in Settings."));
        return;
    }

    QFileInfo cfi(cmakePath);
    if (!cfi.exists())
    {
        QMessageBox::critical(this, tr("CMake Not Found"),
                              tr("CMakeLists.txt does not exist at:\n%1").arg(cmakePath));
        return;
    }

    // --- Backup existing temp contents ---
    QDir tempDir(tempRoot);
    QStringList tempEntries = tempDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);

    if (!tempEntries.isEmpty())
    {
        int ret = QMessageBox::warning(this, tr("Clear Temp Folder"),
                                       tr("This will delete all contents in the temp folder:\n%1\n"
                                          "A backup will be created before deletion.\nContinue?")
                                           .arg(tempRoot),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes)
        {
            queueStatusMessage(tr("Load cancelled."), 2000);
            return;
        }

        // Create timestamped backup folder
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QString backupFolder = backupRoot + "/" + timestamp;
        QDir().mkpath(backupFolder);

        // Progress bar for backup
        QProgressBar *backupProgress = new QProgressBar(this);
        backupProgress->setRange(0, tempEntries.size());
        statusBar()->addPermanentWidget(backupProgress);
        queueStatusMessage(tr("Backing up temp folder..."));

        int backupStep = 0;
        for (int i = 0; i < tempEntries.size(); ++i)
        {
            QString entry = tempEntries.at(i);
            if (entry == "build") { continue; }
            QString srcPath = tempDir.absoluteFilePath(entry);
            QString dstPath = backupFolder + "/" + entry;
            copyRecursively(srcPath, dstPath);
            ++backupStep;
            backupProgress->setValue(backupStep);
            QApplication::processEvents();
        }
        statusBar()->removeWidget(backupProgress);
        backupProgress->deleteLater();
        queueStatusMessage(tr("Backup created: %1").arg(backupFolder), 3000);

        // Progress bar for deletion
        QProgressBar *deleteProgress = new QProgressBar(this);
        deleteProgress->setRange(0, tempEntries.size());
        statusBar()->addPermanentWidget(deleteProgress);
        queueStatusMessage(tr("Deleting temp folder contents..."));

        int deleteStep = 0;
        for (int i = 0; i < tempEntries.size(); ++i)
        {
            QString entry = tempEntries.at(i);
            if (entry == "build") { continue; }
            QString entryPath = tempDir.absoluteFilePath(entry);
            QFileInfo fi(entryPath);
            if (fi.isDir())
            {
                QDir(entryPath).removeRecursively();
            }
            else
            {
                QFile::remove(entryPath);
            }
            ++deleteStep;
            deleteProgress->setValue(deleteStep);
            QApplication::processEvents();
        }
        statusBar()->removeWidget(deleteProgress);
        deleteProgress->deleteLater();
    }

    // --- Copy CMake project files/folders to temp (excluding build) ---
    QDir srcDir = cfi.dir();
    QStringList srcEntries = srcDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);

    QProgressBar *copyProgress = new QProgressBar(this);
    copyProgress->setRange(0, srcEntries.size());
    statusBar()->addPermanentWidget(copyProgress);
    queueStatusMessage(tr("Copying project to temp..."));

    int copyStep = 0;
    for (int i = 0; i < srcEntries.size(); ++i)
    {
        QString entry = srcEntries.at(i);
        if (entry == "build") { continue; }
        QString srcPath = srcDir.absoluteFilePath(entry);
        QString dstPath = tempRoot + "/" + entry;
        copyRecursively(srcPath, dstPath);
        ++copyStep;
        copyProgress->setValue(copyStep);
        QApplication::processEvents();
    }
    statusBar()->removeWidget(copyProgress);
    copyProgress->deleteLater();

    queueStatusMessage(tr("Project loaded to Temp: %1").arg(tempRoot), 3000);
}

/******************************************************************************
 * @brief Recursively copies files and folders from srcPath to dstPath.
 ******************************************************************************/
bool MainWindow::copyRecursively(const QString &srcPath, const QString &dstPath)
{
    QFileInfo srcInfo(srcPath);
    if (srcInfo.isDir())
    {
        QDir().mkpath(dstPath);
        QDir srcDir(srcPath);
        QStringList subEntries = srcDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
        for (int i = 0; i < subEntries.size(); ++i)
        {
            const QString &subEntry = subEntries.at(i);
            QString subSrcPath = srcDir.absoluteFilePath(subEntry);
            QString subDstPath = dstPath + "/" + subEntry;
            if (!copyRecursively(subSrcPath, subDstPath))
            {
                return false;
            }
        }
    }
    else
    {
        QDir().mkpath(QFileInfo(dstPath).absolutePath());
        if (!QFile::copy(srcPath, dstPath))
        {
            QMessageBox::warning(this, tr("Copy Error"),
                                 tr("Could not copy file:\n%1\nto\n%2").arg(srcPath, dstPath));
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * @brief Clear Temp folder contents recursively.
 ******************************************************************************/
void MainWindow::clearTempFolder()
{
    QString root = currentTempRoot();
    auto ret = QMessageBox::warning(this, tr("Clear Temp"),
                                    tr("Delete all contents of:\n%1\nContinue?").arg(root),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) { return; }
    QDir dir(root);
    for (const QFileInfo &fi : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries))
    {
        if (fi.isDir())
        {
            QDir(fi.absoluteFilePath()).removeRecursively();
        }
        else
        {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

/******************************************************************************
 * @brief Extracts the project name from a CMakeLists.txt file.
 *        Looks for a line like: project(MyProject)
 * @param cmakePath Absolute path to CMakeLists.txt.
 * @return Project name, or "DiffCheckAI" if not found.
 ******************************************************************************/
QString MainWindow::extractProjectNameFromCMake(const QString &cmakePath) const
{
    QFile file(cmakePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "DiffCheckAI"; // fallback if file can't be opened

    QString contents = QString::fromUtf8(file.readAll());
    QRegularExpression re(R"(project\s*\(\s*([^\s\)]+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(contents);
    if (match.hasMatch())
        return match.captured(1);

    return "DiffCheckAI"; // fallback if not found
}

/******************************************************************************
 * @brief Returns the temp folder path, one level up from project root,
 *        named after the project (e.g., PROJECTNAME_Temp).
 * @return Absolute path to temp folder.
 ******************************************************************************/
QString MainWindow::currentTempRoot() const
{
    QString cmakePath = appSettings->value("paths/cmake", "").toString();
    if (cmakePath.isEmpty())
        return QDir::homePath() + "/DiffCheckAI_Temp";

    QFileInfo cmakeFile(cmakePath);
    QDir cmakeDir = cmakeFile.dir();
    cmakeDir.cdUp();

    QString projectName = extractProjectNameFromCMake(cmakePath);
    QString tempPath = cmakeDir.absolutePath() + "/" + projectName + "_Temp";
    return tempPath;
}

/******************************************************************************
 * @brief Returns the backup folder path, one level up from project root,
 *        named after the project (e.g., PROJECTNAME_Backups).
 * @return Absolute path to backup folder.
 ******************************************************************************/
QString MainWindow::currentBackupRoot() const
{
    QString cmakePath = appSettings->value("paths/cmake", "").toString();

    if (cmakePath.isEmpty())
        return QDir::homePath() + "/DiffCheckAI_Backups";

    QFileInfo cmakeFile(cmakePath);
    QDir cmakeDir = cmakeFile.dir();
    cmakeDir.cdUp();

    QString projectName = extractProjectNameFromCMake(cmakePath);
    QString backupPath = cmakeDir.absolutePath() + "/" + projectName + "_Backups";
    return backupPath;
}

/******************************************************************************
 * @brief Ensure Temp root folder exists.
 ******************************************************************************/
void MainWindow::ensureTempRoot()
{
    QDir().mkpath(currentTempRoot());
}

/******************************************************************************
 * @brief Copy CMakeLists and sibling sources into Temp (heuristic).
 ******************************************************************************/
void MainWindow::importCMakeToTemp(const QString &cmakePath, const QString &tempRoot)
{
    QFileInfo cfi(cmakePath);
    if (!cfi.exists())
    {
        QMessageBox::critical(this, tr("CMake Not Found"), tr("Missing: %1").arg(cmakePath));
        return;
    }
    QDir srcDir = cfi.dir();
    QDir().mkpath(tempRoot);
    QFile::copy(cfi.absoluteFilePath(), tempRoot + "/CMakeLists.txt");
    QStringList exts{ "*.h", "*.hpp", "*.c", "*.cpp", "*.cc" };
    for (const QString &ext : exts)
    {
        for (const QFileInfo &fi : srcDir.entryInfoList(QStringList(ext), QDir::Files))
        {
            QFile::copy(fi.absoluteFilePath(), tempRoot + "/" + fi.fileName());
        }
    }
}

/******************************************************************************
 * @brief Minimal CMake scanning placeholder; returns matches.
 ******************************************************************************/
QVector<QString> MainWindow::gatherProjectFilesFromCMake(const QString &cmakePath)
{
    QVector<QString> files;
    QFile f(cmakePath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cm = QString::fromUtf8(f.readAll());
        QRegularExpression re(R"(((add_executable|qt_add_executable)\s*\([^\)]*\)))",
                              QRegularExpression::DotMatchesEverythingOption);
        auto it = re.globalMatch(cm);
        while (it.hasNext())
        {
            auto m = it.next();
            files.push_back(m.captured(0));
        }
    }
    return files;
}

/******************************************************************************
 * @brief Saves the contents of the temp file editor to the currently selected file.
 ******************************************************************************/
void MainWindow::actionSaveTempFile()
{
    QModelIndex idx = tempTree->currentIndex();
    if (!idx.isValid())
    {
        QMessageBox::warning(this, tr("No File Selected"), tr("Please select a file in the Temp folder tree."));
        return;
    }
    QString path = tempModel->filePath(idx);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Save Failed"), tr("Cannot write to file:\n%1").arg(path));
        return;
    }
    file.write(tempFileEdit->toPlainText().toUtf8());
    file.close();
    queueStatusMessage(tr("Saved: %1").arg(path), 2000);
}

/******************************************************************************
 * @brief Copies the contents of the temp file editor to the clipboard.
 ******************************************************************************/
void MainWindow::actionCopyTempFile()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(tempFileEdit->toPlainText());
    queueStatusMessage(tr("Copied to clipboard."), 2000);
}

/******************************************************************************
 * @brief Pastes clipboard contents into the temp file editor.
 ******************************************************************************/
void MainWindow::actionPasteTempFile()
{
    QClipboard *clipboard = QApplication::clipboard();
    tempFileEdit->insertPlainText(clipboard->text());
    queueStatusMessage(tr("Pasted from clipboard."), 2000);
}

/******************************************************************************
 * @brief Adds a new project to the projects list.
 ******************************************************************************/
void MainWindow::actionAddProject()
{
    QString name = projectNameEdit->text().trimmed();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, tr("No Project Name"), tr("Please enter a project name."));
        return;
    }
    QList<QListWidgetItem *> items = projectsList->findItems(name, Qt::MatchExactly);
    if (!items.isEmpty())
    {
        QMessageBox::warning(this, tr("Duplicate Project"), tr("Project already exists."));
        return;
    }
    projectsList->addItem(name);
    queueStatusMessage(tr("Project added: %1").arg(name), 2000);
}

/******************************************************************************
 * @brief Deletes the selected project from the projects list.
 ******************************************************************************/
void MainWindow::actionDeleteProject()
{
    QListWidgetItem *item = projectsList->currentItem();
    if (!item)
    {
        QMessageBox::warning(this, tr("No Project Selected"), tr("Please select a project to delete."));
        return;
    }
    delete item;
    queueStatusMessage(tr("Project deleted."));

}

/******************************************************************************
 * @brief Loads the selected project (placeholder for future logic).
 ******************************************************************************/
void MainWindow::actionLoadProject()
{
    QListWidgetItem *item = projectsList->currentItem();
    if (!item)
    {
        QMessageBox::warning(this, tr("No Project Selected"), tr("Please select a project to load."));
        return;
    }
    QString name = item->text();
    // TODO: Implement actual project loading logic
    queueStatusMessage(tr("Loaded project: %1").arg(name), 2000);
}

#include <QLabel>
#include <QColor>
#include <QTimer>
#include <tuple>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

/****************************************************************
 * @brief Queues a colored status bar message with a custom delay.
 * @param message The message to display.
 * @param timeoutMs The display duration in milliseconds.
 * @param color The color of the message text.
 ***************************************************************/
void MainWindow::queueStatusMessage(const QString &message, int timeoutMs, const QColor &color)
{
    {
        std::lock_guard<std::mutex> lock(statusQueueMutex);
        if (statusQueue.size() >= statusQueueMaxSize)
        {
            statusQueue.pop();
        }
        statusQueue.push(std::make_tuple(message, timeoutMs, color));
    }
    statusQueueCV.notify_one();
}

/****************************************************************
 * @brief Worker thread for status message queue.
 ***************************************************************/
void MainWindow::statusQueueWorker()
{
    while (true)
    {
        QString nextMessage;
        int nextTimeout = 2000;
        QColor nextColor = Qt::black;
        {
            std::unique_lock<std::mutex> lock(statusQueueMutex);
            statusQueueCV.wait(lock, [this]
                               {
                                   return !statusQueue.empty() || statusQueueStop;
                               });

            if (statusQueueStop && statusQueue.empty())
            {
                break;
            }

            auto item = statusQueue.front();
            nextMessage = std::get<0>(item);
            nextTimeout = std::get<1>(item);
            nextColor = std::get<2>(item);
            statusQueue.pop();
        }

        QMetaObject::invokeMethod(this, [this, nextMessage, nextTimeout, nextColor]()
                                  {
                                      showStatusBarMessage(nextMessage, nextTimeout, nextColor);
                                  }, Qt::QueuedConnection);

        std::this_thread::sleep_for(std::chrono::milliseconds(nextTimeout));
    }
}

/****************************************************************
 * @brief Shows a colored message on the status bar.
 * @param message The message to display.
 * @param timeoutMs The display duration in milliseconds.
 * @param color The color of the message text.
 ***************************************************************/
void MainWindow::showStatusBarMessage(const QString &message, int timeoutMs, const QColor &color)
{
    if (!colorLabel)
    {
        colorLabel = new QLabel(this);
        statusBar()->addPermanentWidget(colorLabel, 1);
    }
    colorLabel->setText(message);

    QPalette palette = colorLabel->palette();
    palette.setColor(QPalette::WindowText, color);
    colorLabel->setPalette(palette);

    // Optionally clear after timeout
    QTimer::singleShot(timeoutMs, this, [this]()
                       {
                           if (colorLabel)
                           {
                               colorLabel->clear();
                           }
                       });
}

void MainWindow::importSettings(const QString& filePath)
{
    if (appSettings->import(filePath))
        queueStatusMessage("Settings imported!", 3000, Qt::green);
    else
        queueStatusMessage("Import failed!", 3000, Qt::red);
}

void MainWindow::exportSettings(const QString& filePath)
{
    if (appSettings->exportTo(filePath))
        queueStatusMessage("Settings exported!", 3000, Qt::green);
    else
        queueStatusMessage("Export failed!", 3000, Qt::red);
}

/****************************************************************
 * @brief Slot called when the CMake path is changed.
 * Enables/disables temp and backup path edits and sets defaults.
 ***************************************************************/
void MainWindow::onCmakePathChanged()
{
    QString cmakePath = cmakePathEdit->text().trimmed();
    QFileInfo cmakeFileInfo(cmakePath);

    if (cmakeFileInfo.exists() && cmakeFileInfo.isFile())
    {
        QString appName = QCoreApplication::applicationName();
        QDir cmakeDir = cmakeFileInfo.dir();
        cmakeDir.cdUp();

        QString tempPathDefault = cmakeDir.absolutePath() + "/" + appName + "_Temp";
        QString backupPathDefault = cmakeDir.absolutePath() + "/" + appName + "_Backups";

        tempPathEdit->setText(appSettings->value("paths/temp", tempPathDefault).toString());
        backupPathEdit->setText(appSettings->value("paths/backup", backupPathDefault).toString());

        tempPathEdit->setEnabled(true);
        backupPathEdit->setEnabled(true);
    }
    else
    {
        tempPathEdit->setEnabled(false);
        backupPathEdit->setEnabled(false);
        tempPathEdit->clear();
        backupPathEdit->clear();
    }
}
/*************** End of MainWindow.cpp ***************************************/
