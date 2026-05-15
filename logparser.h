#pragma once
#include <qstring>
#include <qdebug>
#include <utils.h>

class LogParser: public QObject
{
	Q_OBJECT
public:
	explicit LogParser(QObject* parent = nullptr);
public slots:
	void onRawLineReceived(const QString& line);

signals:
	void entryParsed(const LogEntry& entry);

};
