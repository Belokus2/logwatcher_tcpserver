#include "filewatcher.h"
#include <QFile>
#include <QTextStream>

FileWatcher::FileWatcher(const QString& dirPath, QObject* parent)
	: QObject(parent), m_directory(dirPath), m_timer(new QTimer(this))
{
	//Устанавливаем фильтры по расширениям
	m_filters << "*.log" << "*.txt" << "*.csv";

	connect(m_timer, &QTimer::timeout, this, &FileWatcher::onCheckTimer);
	m_timer->start(500); //500мс достаточно для проверки папки
}

void FileWatcher::onCheckTimer()
{
	m_directory.refresh(); //Обновляем состояние папки

	//Получаем список файлов, подходящих под фильтры
	QStringList files = m_directory.entryList(m_filters, QDir::Files);

	for (const QString& fileName : files) {
		QString fullPath = m_directory.absoluteFilePath(fileName);
		QFile file(fullPath);

		qint64 fileSize = file.size();
		qint64 lastPos = m_filePositions.value(fullPath, 0);

		//Если файл уменьшился (перетерт), сбрасываем позицию
		if (fileSize < lastPos) lastPos = 0;
		//Если появились новые данные
		if (fileSize > lastPos) {
			if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				file.seek(lastPos);
				QTextStream in(&file);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if (!line.isEmpty()) emit lineReady(line);
				}
				m_filePositions[fullPath] = file.pos();
				file.close();
			}
		}
	}
}