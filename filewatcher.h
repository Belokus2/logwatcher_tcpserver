#pragma once
#include <QObject>
#include <QDir>
#include <QMap>
#include <QTimer>
#include <QStringList>

class FileWatcher : public QObject
{
	Q_OBJECT
public:
	//Теперь принимаем путь к папке
	explicit FileWatcher(const QString& dirPath, QObject* parent = nullptr);

private:
	QDir m_directory;
	//Карта:"Путь к файлу" -> "Последняя позиция чтения"
	QMap<QString, qint64> m_filePositions;
	QTimer* m_timer;
	QStringList m_filters; //Список расширений (.log, .txt, .csv)

signals:
	void lineReady(const QString& line);

private slots:
	void onCheckTimer();
};