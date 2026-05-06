#pragma once
#include <qfile>
#include <qstring>
#include <qfilesystemwatcher>
#include <qdebug>
#include <qtimer>

class FileWatcher : public QObject
{
	Q_OBJECT
public:
	explicit FileWatcher(const QString& path, QObject* parent = nullptr);

private:
	QString m_filePath;
	QFile m_logFile;
	qint64 m_lastPosition;
	QTimer* m_timer;
signals:
	void lineReady(const QString& line);

private slots:
	void onCheckTimer();
};
