#include "niqamainwindow.h"
#include <filesystem>
#include <QApplication>
#include <QDir>
#include <logging/logger.h>
#include <strings/miscstring.h>
#include <strings/filenames.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NIQAMainWindow w;

    std::string homedir = QDir::homePath().toStdString();
    kipl::strings::filenames::CheckPathSlashes(homedir,true);
    kipl::logging::Logger logger("NIQA");
    kipl::logging::Logger::SetLogLevel(kipl::logging::Logger::LogMessage);
    logger.message("Starting NIQA");

    std::string logpath = homedir+".imagingtools";
    if (!std::filesystem::exists(logpath))
    {
      std::filesystem::create_directories(logpath);
    }

    logpath = logpath+ "/niqa.log";
    kipl::strings::filenames::CheckPathSlashes(logpath,false);
    kipl::logging::LogStreamWriter logstream(logpath);
    logger.addLogTarget(&logstream);
    w.show();

    return a.exec();
}
