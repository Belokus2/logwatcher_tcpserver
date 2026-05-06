#pragma once
#include <qtcpserver>
#include <qtcpsocket>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QtGlobal>
#include <qdebug.h>
#include <qtimer.h>
#include "statsengine.h"
#include <QSet>
#include <qjsonarray.h>

class TcpServer : public QTcpServer {
	Q_OBJECT
public:
	explicit TcpServer(StatsEngine& engine, QObject* parent = nullptr);

	bool start(int port);
	void incomingConnection(qintptr socketDescriptor) override;

private slots:
	void onDisconeccted();
	void onReadyRead();
	void onUpdateMetrics();

private:
	void handlePing(QTcpSocket* socket);
	void handleSubscribe(QTcpSocket* socket);
	void handleUnsubscribe(QTcpSocket* socket);
	void handleQuery(QTcpSocket* socket, const QJsonObject& request);
	StatsEngine& m_engine;
	QTimer* checkMetrics;
	QSet<QTcpSocket*> m_subscribers;
};
