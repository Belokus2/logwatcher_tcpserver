#include "logstorage.h"
#include <QDebug>

LogStorage::LogStorage(size_t maxBufferSize, QObject* parent)
	: QObject(parent), m_maxSize(maxBufferSize) {
}

void LogStorage::onEntryParsed(const LogEntry& entry) {
	m_recentLogs.push_back(entry);

	if (m_recentLogs.size() > m_maxSize) {
		m_recentLogs.pop_front();
	}

	m_levelStats[entry.level]++;

	QString timeStr = entry.timestamp.toString("dd.MM.yyyy HH:mm:ss.zzz");
	QString levelStr = entry.level.leftJustified(8, ' ');
	QString sourceStr = entry.source.leftJustified(15, ' ');
	QString codeStr = QString::number(entry.code).leftJustified(5, ' ');

	qDebug().noquote() << QString("[%1] | %2 | %3 | Code: %4 | %5")
		.arg(timeStr)
		.arg(levelStr)
		.arg(sourceStr)
		.arg(codeStr)
		.arg(entry.message);
}

int LogStorage::getLevelCount(const QString& level) const {
	auto it = m_levelStats.find(level);
	if (it != m_levelStats.end()) {
		return it->second;
	}
	return 0;
}