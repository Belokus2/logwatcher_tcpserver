#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include "tcpserver.h"
#include "filewatcher.h"
#include "logparser.h"
#include "logstorage.h"
#include "statsengine.h"
#include <QTextCodec>
#include <windows.h> // Добавь этот инклуд к остальным

int main(int argc, char* argv[])
{

	SetConsoleOutputCP(CP_UTF8);
	// Для корректного ввода из консоли (если понадобится)
	SetConsoleCP(CP_UTF8);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	QCoreApplication app(argc, argv);

	QString path = "C:/Users/getto/AppData/Local/Temp/LogWatchTest/example.log";

	qDebug() << "start";
	qDebug() << "Monitoring file: " << QDir::toNativeSeparators(path);

	FileWatcher watcher(path);
	LogParser parser;
	LogStorage storage(5000);

	StatsEngine engine(storage, 0.2, 5);
	TcpServer server(engine);

	QObject::connect(&watcher, &FileWatcher::lineReady, &parser, &LogParser::onRawLineReceived);
	QObject::connect(&parser, &LogParser::entryParsed, &storage, &LogStorage::onEntryParsed);
	QObject::connect(&parser, &LogParser::entryParsed, &engine, &StatsEngine::onNewLogAdded);

	int port = 8080;
	if (!server.start(port)) {
		qCritical() << "Не удалось запустить сервер! Завершение работы.";
		return -1;
	}

	return app.exec();
}