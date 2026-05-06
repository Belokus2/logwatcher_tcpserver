#pragma once
#include <QString>
#include <QDateTime>

enum class GetCommand {};

struct LogEntry
{
	QDateTime timestamp;
	QString level;
	QString source;
	int code;
	QString message;
};
