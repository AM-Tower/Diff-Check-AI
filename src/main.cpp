/******************************************************************************
 * @file main.cpp
 * @brief Application entry point for CodeHelpAI.
 *
 * @author Jeffrey Scott Flesher with the help of AI: Copilot
 * @version 0.8
 * @date 2025-11-07
 * @section License MIT
 * @section DESCRIPTION
 * Loads resources and translations, constructs MainWindow,
 * sets app icon and shows the window.
 ******************************************************************************/

#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
//#define SHOW_DEBUG 1
//#include "Config.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    #ifdef __MINGW32__
    // Force Qt 6 to use OpenGL instead of D3D12
    qputenv("QT_DEFAULT_RHI", "opengl");
    qDebug() << "QT_DEFAULT_RHI =" << qEnvironmentVariable("QT_DEFAULT_RHI");
    qputenv("QT_QPA_PLATFORM", "windows");
    qDebug() << "QT_QPA_PLATFORM =" << qEnvironmentVariable("QT_QPA_PLATFORM");
    #endif

    QApplication app(argc, argv);

    // Register resources
    Q_INIT_RESOURCE(DiffCheckAI);

    // App icon
    QApplication::setWindowIcon(QIcon(":/icons/icons/app.svg"));

    // Load translations based on system locale
    const QString langCode = QLocale::system().name().split('_').first();
    auto loadTr = [&](const QString &base) -> bool
    {
        auto *tr = new QTranslator(&app);
        const QString qm = QString(":/translations/%1_%2.qm").arg(base, langCode);
        if (tr->load(qm))
        {
            app.installTranslator(tr);
            return true;
        }
        delete tr;
        return false;
    };
    loadTr("CodeHelpAI");

    MainWindow w;
    w.show();

    return app.exec();
}

/*************** End of main.cpp ***************************************/
