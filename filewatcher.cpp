#include "filewatcher.h"

FileWatcher::FileWatcher(const QString& path, QObject* parent)
	: m_filePath(path), m_lastPosition(0), m_timer(new QTimer(this)), QObject(parent)
{
	m_logFile.setFileName(m_filePath);

	connect(m_timer, &QTimer::timeout, this, &FileWatcher::onCheckTimer);

	m_timer->start(100);

}


void FileWatcher::onCheckTimer()
{
	qint64 fileSize = m_logFile.size();
	if (fileSize == m_lastPosition) return;
	if (fileSize < m_lastPosition) m_lastPosition = 0;
	if (m_logFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		m_logFile.seek(m_lastPosition);

		QTextStream in(&m_logFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (!line.isEmpty()) emit lineReady(line);
		}
		m_lastPosition = m_logFile.pos();
		m_logFile.close();
	}
}