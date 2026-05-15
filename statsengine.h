#pragma once
#include <qobject.h>
#include <qdatetime.h>
#include <deque>
#include <map>
#include <qstring.h>
#include "logstorage.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <qstring.h>

struct TrendLine {
	double k = 0.0;
	double b = 0.0;
};

struct LevelStats
{
	std::deque<QDateTime> slidingWindow;
	double currentEma = 0.0;

	QDateTime lastLogTime;
	std::deque<qint64> recentIntervals;

	std::deque<double> trendHistoryY;

	std::deque<QString> recentLogs;
};

class StatsEngine : public QObject
{
	Q_OBJECT
public:
	explicit StatsEngine(const LogStorage& storage, double alpha = 0.2, int windowSizeSec = 60, QObject* parent = nullptr);
	QStringList getLastNLogs(const QString& level, int n) const;
	double getEmaForLevel(const QString& level) const;
	double calculatePercentile(const QString level, double percent) const;
	TrendLine calculateTrend(const std::deque<double>& yValues) const;
	void calculateEma(const QDateTime& currentSystemTime);
	TrendLine getTrendForLevel(const QString& level) const;
	double predictForecast(const QString& level, int futureSteps) const;

public slots:
	void onNewLogAdded(const LogEntry& entry);

private:
	void addNewLog(const QString& level, const QDateTime& timestamp);

	std::map<QString, LevelStats> m_emaStats;
	double m_alpha;
	int m_windowSizeSec;
	std::map<QString, TrendLine> m_levelTrends;
};
