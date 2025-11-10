/****************************************************************
 * @file    test_settings.cpp
 * @brief   Unit tests for MainWindow Settings tab.
 *
 * @author  Jeffrey Scott Flesher with the help of AI: Copilot
 * @version 0.4
 * @date    2025-11-09
 * @section License MIT
 * @section DESCRIPTION
 * Tests Settings tab buttons: browse, save, and path validation.
 * Automatically saves and restores QSettings for each test.
 ***************************************************************/

#include <QtTest>
#include "MainWindow.h"

class TestSettingsTab : public QObject
{
    Q_OBJECT
private:
    QVariant origCmake, origTemp, origBackup;

private slots:
    void init()
    {
        QSettings settings("AM-Tower", "DiffCheckAI");
        origCmake = settings.value("paths/cmake");
        origTemp = settings.value("paths/temp");
        origBackup = settings.value("paths/backup");
    }

    void cleanup()
    {
        QSettings settings("AM-Tower", "DiffCheckAI");
        settings.setValue("paths/cmake", origCmake);
        settings.setValue("paths/temp", origTemp);
        settings.setValue("paths/backup", origBackup);
    }

    void testSaveButton()
    {
        MainWindow w;
        QLineEdit* cmakeEdit = w.findChild<QLineEdit*>("cmakePathEdit");
        QVERIFY2(cmakeEdit, "cmakePathEdit not found");
        QLineEdit* tempEdit = w.findChild<QLineEdit*>("tempPathEdit");
        QVERIFY2(tempEdit, "tempPathEdit not found");
        QLineEdit* backupEdit = w.findChild<QLineEdit*>("backupPathEdit");
        QVERIFY2(backupEdit, "backupPathEdit not found");

        // Create dummy files/folders in the build/test working directory
        QString cmakePath = QDir::currentPath() + "/CMakeLists.txt";
        QFile dummyCMake(cmakePath);
        if (!dummyCMake.exists())
        {
            if (dummyCMake.open(QIODevice::WriteOnly))
            {
                dummyCMake.write("cmake_minimum_required(VERSION 3.24)\n");
                dummyCMake.close();
            }
            else
            {
                QFAIL("Failed to create dummy CMakeLists.txt file for test.");
            }
        }
        QString tempPath = QDir::currentPath() + "/temp";
        QDir().mkpath(tempPath);
        QString backupPath = QDir::currentPath() + "/backup";
        QDir().mkpath(backupPath);

        // Use relative paths for the test
        cmakeEdit->setText("CMakeLists.txt");
        tempEdit->setText("temp");
        backupEdit->setText("backup");

        // Test saveSettings return value
        QVERIFY(w.saveSettings() == true);

        QSettings settings("AM-Tower", "DiffCheckAI");
        QCOMPARE(settings.value("paths/cmake").toString(), "CMakeLists.txt");
        QCOMPARE(settings.value("paths/temp").toString(), "temp");
        QCOMPARE(settings.value("paths/backup").toString(), "backup");
    }

    void testPathValidation()
    {
        MainWindow w;
        w.findChild<QLineEdit*>("cmakePathEdit")->setText("/invalid/path.txt");
        w.findChild<QLineEdit*>("tempPathEdit")->setText("/invalid/temp");
        w.findChild<QLineEdit*>("backupPathEdit")->setText("/invalid/backup");

        // Should fail for invalid paths
        QVERIFY(w.saveSettings() == false);
    }
};

QTEST_MAIN(TestSettingsTab)
#include "test_settings.moc"

/************** End of test_settings.cpp **************************/
