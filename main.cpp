#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QTextCodec>
#include <windows.h> // Возвращаем библиотеку для работы с кодировкой консоли

#include "tcpserver.h"
#include "filewatcher.h"
#include "logparser.h"
#include "logstorage.h"
#include "statsengine.h"

int main(int argc, char* argv[])
{
	// Возвращаем настройки кодировки UTF-8 для Windows
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	QCoreApplication app(argc, argv);

	QCommandLineParser parser;
	parser.setApplicationDescription("Log Watcher Server");
	parser.addHelpOption();

	// Опция для директории
	QCommandLineOption dirOption(QStringList() << "d" << "dir", "Путь к директории с логами", "directory");
	parser.addOption(dirOption);

	// Опция для порта
	QCommandLineOption portOption(QStringList() << "p" << "port", "Порт", "port", "8080");
	parser.addOption(portOption);

	parser.process(app);

	// Проверка директории
	QString dirPath = parser.value(dirOption);
	if (dirPath.isEmpty() || !QDir(dirPath).exists()) {
		qCritical() << "Ошибка: Директория не найдена или не указана (-d)";
		return 1;
	}

	// Проверка порта
	bool ok;
	int port = parser.value(portOption).toInt(&ok);
	if (!ok || port <= 0) {
		qCritical() << "Ошибка: Некорректный порт (-p)";
		return 1;
	}

	FileWatcher watcher(dirPath); // Следим за всей папкой
	LogParser logParser;
	LogStorage storage(5000);
	StatsEngine engine(storage);
	TcpServer server(engine);

	// Соединяем компоненты
	QObject::connect(&watcher, &FileWatcher::lineReady, &logParser, &LogParser::onRawLineReceived);
	QObject::connect(&logParser, &LogParser::entryParsed, &storage, &LogStorage::onEntryParsed);
	QObject::connect(&logParser, &LogParser::entryParsed, &engine, &StatsEngine::onNewLogAdded);

	if (!server.start(port)) return -1;

	qInfo() << "Мониторинг директории:" << dirPath;
	return app.exec();
}