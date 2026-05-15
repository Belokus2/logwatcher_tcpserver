#include "statsengine.h"

StatsEngine::StatsEngine(const LogStorage& storage, double alpha, int windowSizeSec, QObject* parent)
	: QObject(parent), m_alpha(alpha), m_windowSizeSec(windowSizeSec)
{
	auto& history = storage.getRecentLogs();
	for (const auto& log : history) {
		addNewLog(log.level, log.timestamp);
		addNewLog("all", log.timestamp); //Добавляем поддержку уровня "all" для исторических данных
	}
}

void StatsEngine::onNewLogAdded(const LogEntry& entry) {
	//Формируем полную строку лога вместо сохранения только сообщения
	QString fullLog = QString("[%1] | %2 | %3 | Code: %4 | %5")
		.arg(entry.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"))
		.arg(entry.level)
		.arg(entry.source)
		.arg(entry.code)
		.arg(entry.message);

	//Лямбда-выражение для обновления статистики конкретного уровня
	auto updateLevelStats = [&](const QString& targetLevel) {
		addNewLog(targetLevel, entry.timestamp);
		auto& stats = m_emaStats[targetLevel];

		if (stats.lastLogTime.isValid()) {
			qint64 interval = stats.lastLogTime.msecsTo(entry.timestamp);
			stats.recentIntervals.push_back(interval);

			if (stats.recentIntervals.size() > 1000) {
				stats.recentIntervals.pop_front();
			}
		}
		stats.lastLogTime = entry.timestamp;

		//Добавляем в буфер весь сформированный лог
		stats.recentLogs.push_back(fullLog);
		if (stats.recentLogs.size() > 100) stats.recentLogs.pop_front();
		};

	//Обновляем статистику для конкретного уровня (например, "Error")
	updateLevelStats(entry.level);

	// Обновляем общую статистику для запросов level = "all"
	updateLevelStats("all");
}

QStringList StatsEngine::getLastNLogs(const QString& level, int n) const
{
	QStringList result;
	auto it = m_emaStats.find(level);

	if (it == m_emaStats.end()) return result;
	const auto& logsDeque = it->second.recentLogs;
	int totalLogs = logsDeque.size();

	int startIndex = std::max(0, totalLogs - n);

	for (int i = startIndex; i < totalLogs; ++i)
	{
		result.append(logsDeque[i]);
	}
	return result;
}

double StatsEngine::getEmaForLevel(const QString& level) const
{
	auto it = m_emaStats.find(level);
	if (it != m_emaStats.end()) { return it->second.currentEma; }

	return 0.0;
}

double StatsEngine::calculatePercentile(const QString level, double percent) const
{
	auto it = m_emaStats.find(level);
	if (it == m_emaStats.end()) return 0.0;
	const auto& intervals = it->second.recentIntervals;
	if (intervals.empty()) return 0.0;

	std::vector<qint64> sortedIntervals(intervals.begin(), intervals.end());
	std::sort(sortedIntervals.begin(), sortedIntervals.end());

	double p = percent / 100.0;
	double index = ceil(p * sortedIntervals.size()) - 1;

	return sortedIntervals[index];
}

TrendLine StatsEngine::calculateTrend(const std::deque<double>& yValues) const
{
	TrendLine result;
	int n = yValues.size();
	if (yValues.size() < 2)
	{
		result.k = 0.0;
		result.b = (n == 1) ? yValues.front() : 0.0;
		return result;
	}
	double sumX = 0.0;
	double sumY = 0.0;
	double sumXY = 0.0;
	double sumX2 = 0.0;

	for (int i = 0; i < n; ++i)
	{
		double x = static_cast<double>(i);
		double y = yValues[i];
		sumX += x;
		sumY += y;
		sumXY += x * y;
		sumX2 += x * x;
	}

	double denominator = (n * sumX2) - (sumX * sumX);

	if (denominator == 0.0)
	{
		result.k = 0.0;
		result.b = sumY / n;
		return result;
	}

	result.k = (n * sumXY - sumX * sumY) / denominator;
	result.b = (sumY - result.k * sumX) / n;
	return result;
}

void StatsEngine::addNewLog(const QString& level, const QDateTime& timestamp)
{
	auto& stats = m_emaStats[level];
	stats.slidingWindow.push_back(timestamp);
}

void StatsEngine::calculateEma(const QDateTime& currentSystemTime)
{
	for (auto it = m_emaStats.begin(); it != m_emaStats.end(); ++it)
	{
		QString level = it->first;
		auto& stats = it->second;
		while (!stats.slidingWindow.empty())
		{
			QDateTime oldestTimestamp = stats.slidingWindow.front();
			if (oldestTimestamp.secsTo(currentSystemTime) > m_windowSizeSec) stats.slidingWindow.pop_front();
			else break;
		}
		double currentFrequency = static_cast<double>(stats.slidingWindow.size()) / m_windowSizeSec;
		if (stats.currentEma == 0 && currentFrequency > 0) stats.currentEma = currentFrequency;
		else stats.currentEma = (currentFrequency * m_alpha) + (stats.currentEma * (1 - m_alpha));
		if (stats.currentEma < 0.1) stats.currentEma = 0;

		stats.trendHistoryY.push_back(stats.currentEma);
		if (stats.trendHistoryY.size() > 60) stats.trendHistoryY.pop_front();
		TrendLine currentTrend = calculateTrend(stats.trendHistoryY);
		m_levelTrends[level] = currentTrend;
	}
}

TrendLine StatsEngine::getTrendForLevel(const QString& level) const
{
	auto it = m_levelTrends.find(level);
	if (it != m_levelTrends.end()) return it->second;
	return TrendLine();
}

double StatsEngine::predictForecast(const QString& level, int futureSteps) const
{
	auto it = m_levelTrends.find(level);
	if (it == m_levelTrends.end()) return 0.0;

	auto statsIt = m_emaStats.find(level);
	if (statsIt == m_emaStats.end() || statsIt->second.trendHistoryY.empty()) return 0.0;

	const TrendLine& trend = it->second;
	double futureX = static_cast<double>(statsIt->second.trendHistoryY.size() - 1 + futureSteps);
	return trend.k * futureX + trend.b;
}