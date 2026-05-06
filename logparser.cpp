#include "logparser.h"
#include <qregularexpression>

LogParser::LogParser(QObject* parent) : QObject(parent) {};
void LogParser::onRawLineReceived(const QString& line)
{
	static QRegularExpression re("^(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}),\\s*(\\w+),\\s*(\\w+),\\s*Code:(\\d+),\\s*(.*)$");

	QRegularExpressionMatch match = re.match(line);

	if (match.hasMatch()) {
		LogEntry entry;
		entry.timestamp = QDateTime::fromString(match.captured(1), "yyyy-MM-dd HH:mm:ss.zzz");
		entry.level = match.captured(2);
		entry.source = match.captured(3);
		entry.code = match.captured(4).toInt();
		entry.message = match.captured(5);

		emit entryParsed(entry);
	}
	else {
		qDebug() << "Parser warning: " << line;
	}
}
