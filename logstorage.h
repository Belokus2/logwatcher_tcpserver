#pragma once
#include <QObject>
#include <deque>
#include <unordered_map>
#include "utils.h"

class LogStorage : public QObject {
	Q_OBJECT
public:
	explicit LogStorage(size_t maxBufferSize = 1000, QObject* parent = nullptr);

	// Методы для мат. движка (чтобы он мог забирать данные)
	const std::deque<LogEntry>& getRecentLogs() const { return m_recentLogs; }
	int getLevelCount(const QString& level) const;

public slots:
	void onEntryParsed(const LogEntry& entry);

private:
	size_t m_maxSize;

	// 1. Кольцевой буфер последних событий
	std::deque<LogEntry> m_recentLogs;

	// 2. Агрегация: Считаем, сколько логов каждого уровня мы получили
	// Ключ - "Error", "Info" и т.д., Значение - количество
	std::unordered_map<QString, int> m_levelStats;
};
