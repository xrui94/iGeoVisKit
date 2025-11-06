#include "PchApp.h"

// Removed global header; include only what is necessary per-file

#include "config.h"

#include "imagery/ImageProperties.h"
#include "imagery/BandInfo.h"
#include "imagery/ROISet.h"
#include "gui/FeatureSpace.h"
#include "utils/Settings.h"
#include "utils/StringUtils.h"
#include "utils/Console.h"

#include "gui/MainWindow.h"
#include "gui/ImageWindow.h"
#include "gui/OverviewWindow.h"
#include "gui/ToolWindow.h"
#include "gui/ROIWindow.h"
#include "gui/PrefsWindow.h"
#include "gui/ContrastWindow.h"
#include "gui/ContrastAdvWindow.h"
#include "gui/ProgressStatusWidget.h"
//#include "FileDialogQt.h"

settings* settingsFile = nullptr;

char szStaticControl[] = "static";  /* classname of static text control */

ROISet *regionsSet = nullptr;

MainWindow* mainWindow = nullptr;

// Qt message handler to capture logs into a file (useful for GUI apps without console)
static QFile *gQtLogFile = nullptr;
static QTextStream *gQtLogStream = nullptr;
static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    if (gQtLogStream) {
        (*gQtLogStream) << QDateTime::currentDateTime().toString(Qt::ISODate) << " [" << (int)type << "] " << msg << "\n";
        gQtLogStream->flush();
    }
}

/// <summary>
/// Get the path to this executable's directory 
/// </summary>
static std::string getModulePath()
{
    // 使用 Qt 获取应用程序目录，避免依赖 Windows API
    QString dir = QCoreApplication::applicationDirPath();
    QByteArray bytes = dir.toLocal8Bit();
    std::string modulePath;
    memcpy(modulePath.data(), bytes.constData(), bytes.size());
    //modulePath[bytes.size()] = '\0';
    return modulePath;
}

#ifdef _WIN32


    #ifdef USE_CONSOLE
        static void setupConsole()
        {
            // 1. 分配控制台
            AllocConsole();

            // 2. 设置控制台代码页为 UTF-8
            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8); // 输入也设为 UTF-8（可选）

            // 3. 重定向 stdout/stderr 到控制台
            FILE* f;
            freopen_s(&f, "CONOUT$", "w", stdout);
            freopen_s(&f, "CONOUT$", "w", stderr);

            // 4. 保持文本模式，输出字节流（UTF-8），避免 Qt/stdio 混用崩溃
            _setmode(_fileno(stdout), _O_TEXT);
            _setmode(_fileno(stderr), _O_TEXT);

            // 5. （可选）设置 locale 为 UTF-8
            std::locale::global(std::locale(".UTF-8"));
            std::cout.imbue(std::locale());

            // 6. 测试输出
            std::cout << "✅ 控制台已启用，支持中文：你好，世界！\n";
        }
    #endif  // USE_CONSOLE

#endif  // _WIN32

/* Qt program entry point */
int main(int argc, char *argv[])
{
#if definde(_WIN32) && defined(USE_CONSOLE)
    setupConsole();
#endif  // _WIN32

    // Ensure Qt can locate platform plugins when launched from IDE or different CWD
    QString appDir = QCoreApplication::applicationDirPath();

    // 璁剧疆鍏ㄥ眬榛樿 OpenGL 鏍煎紡涓?3.3 Core Profile
    {
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        fmt.setDepthBufferSize(16);
        fmt.setStencilBufferSize(8);
        QSurfaceFormat::setDefaultFormat(fmt);
    }

    // (Optional) Add exe directory to DLL search path to help plugin dependencies resolve
    // Disabled to avoid API availability issues across Windows SDKs.
    // Install Qt message handler to capture plugin loader logs into a file
    {
        QString logPath;
        if (!appDir.isEmpty()) {
            logPath = appDir + "/qt_log.txt";
        } else {
            logPath = "qt_log.txt"; // fallback to CWD
        }
        gQtLogFile = new QFile(logPath);
        if (gQtLogFile->open(QIODevice::Append | QIODevice::Text)) {
            gQtLogStream = new QTextStream(gQtLogFile);
            // Qt6 默认 UTF-8，但显式设置以避免不同平台差异
        #if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            gQtLogStream->setEncoding(QStringConverter::Utf8);
        #endif
            qInstallMessageHandler(qtMessageHandler);
            (*gQtLogStream) << QDateTime::currentDateTime().toString(Qt::ISODate) << " [INFO] main start, log initialized at: " << logPath << "\n";
            gQtLogStream->flush();
        }
    }

    //qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
    //// 浠呭綋澶栭儴鏈缃?QT_QPA_PLATFORM_PLUGIN_PATH 鏃讹紝鎵嶄娇鐢ㄦ湰鍦?appDir/platforms
    //if (!appDir.isEmpty() && !qEnvironmentVariableIsSet("QT_QPA_PLATFORM_PLUGIN_PATH")) {
    //    QDir platformsDir(appDir + "/platforms");
    //    if (platformsDir.exists()) {
    //        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", platformsDir.absolutePath().toUtf8());
    //    }
    //    // 閬垮厤瑕嗙洊榛樿鎻掍欢鎼滅储璺緞锛屾殏涓嶈缃?QT_PLUGIN_PATH
    //}

    //// Early diagnostic dialog to verify paths before creating QApplication
    //{
    //    const char* envPlat = getenv("QT_QPA_PLATFORM_PLUGIN_PATH");
    //    char diag[1024];
    //    snprintf(diag, sizeof(diag),
    //             "modulePath=%s\nappDir=%s\nQT_QPA_PLATFORM_PLUGIN_PATH=%s\nplatformsExists=%s",
    //             modulePath ? modulePath : "(null)",
    //             appDir.toUtf8().constData(),
    //             envPlat ? envPlat : "(unset)",
    //             QDir(appDir + "/platforms").exists() ? "true" : "false");
    //    MessageBoxA(NULL, diag, "Parbat3D Startup", MB_OK | MB_ICONINFORMATION);
    //}

    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Parbat3D");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Imagery");
    app.setOrganizationDomain("imagery.org");
    
    // Ensure Qt sees correct plugin search paths. If external platform path is set,
    // prioritize it and avoid adding local platforms to prevent mixed binaries.
    if (!appDir.isEmpty()) {
        const bool externalPlatSet = qEnvironmentVariableIsSet("QT_QPA_PLATFORM_PLUGIN_PATH");
        if (externalPlatSet) {
            QStringList libs;
            libs << qEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH");
            libs << appDir; // app resources
            libs << appDir + "/styles";
            libs << appDir + "/imageformats";
            QCoreApplication::setLibraryPaths(libs);
        } else {
            QCoreApplication::addLibraryPath(appDir);
            QCoreApplication::addLibraryPath(appDir + "/platforms");
            QCoreApplication::addLibraryPath(appDir + "/styles");
            QCoreApplication::addLibraryPath(appDir + "/imageformats");
        }
    }
    
    //string settings_path (catcstrings(modulePath, "\\settings.ini"));  // store in build folder
    
    // create the directory in Application Data if it doesn't already exist
    QString settings_path = "settings.ini";
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    if (!dataPaths.isEmpty()) {
        QString team_dir = dataPaths.first() + "/Imagery";
        QDir().mkpath(team_dir);
        QString prog_dir = team_dir + "/Parbat3D";
        QDir().mkpath(prog_dir);
        settings_path = prog_dir + "/settings.ini";  // store in App Data folder
    }
    
    // Used for loading and saving window position and sizes
    // Convert QString to std::string for settingsFile
    std::string settings_path_std = settings_path.toStdString();
    settingsFile = new settings(settings_path_std);
    
    regionsSet = new ROISet();
    
    Console::open();
    Console::write("Console columns: ");
    Console::write((char*)inttocstring(80));  // Default console width
    Console::write("\n");
    Console::write("Console rows: ");
    Console::write((char*)inttocstring(25));  // Default console height
    Console::write("\n");

    // Create main window (after QApplication)
    mainWindow = new MainWindow();
    mainWindow->show();

    // 鍒濆鍖?StickyWindowManager锛堝繀椤诲湪 QApplication 涔嬪悗锛?
    //stickyWindowManager = new StickyWindowManager();
    //stickyWindowManager->SetController(mainWindow);
       
    // Main window already shown above
    
    // Execute the application
    return app.exec();
}
