#include "tcpserver.h"

TcpServer::TcpServer(StatsEngine &engine, QObject *parent)
    : QTcpServer(parent), m_engine(engine) {

  QTimer *checkMetrics = new QTimer(this);
  connect(checkMetrics, &QTimer::timeout, this, &TcpServer::onUpdateMetrics);
  checkMetrics->start(1000);
}

void TcpServer::onUpdateMetrics() {
  m_engine.calculateEma(QDateTime::currentDateTime());
  QString levelToMonitor = "Error";
  double currentEma = m_engine.getEmaForLevel(levelToMonitor);
  if (currentEma > 5) {
    QJsonObject pushAlert;
    pushAlert["event"] = "ALERT";
    pushAlert["level"] = levelToMonitor;
    pushAlert["message"] = "Критический рост ошибок!";
    pushAlert["current_ema"] = currentEma;
    pushAlert["forecast"] = m_engine.predictForecast(levelToMonitor, 5);

    QByteArray alertData =
        QJsonDocument(pushAlert).toJson(QJsonDocument::Compact) + "\n";
    for (QTcpSocket *subSocket : qAsConst(m_subscribers)) {
      if (subSocket->state() == QAbstractSocket::ConnectedState) {
        subSocket->write(alertData);
      }
    }
  }
}

bool TcpServer::start(int port) {
  if (this->listen(QHostAddress::Any, port)) {
    qInfo() << "Сервер запущен на порте: " << port;
    return true;
  } else {
    qInfo() << "Что-то пошло не так: " << this->errorString();
    return false;
  }
}

void TcpServer::incomingConnection(qintptr socketDescriptor) {
  qInfo() << "Входящее подключение. Дескриптор: " << socketDescriptor;
  QTcpSocket *socket = new QTcpSocket(this);

  if (socket->setSocketDescriptor(socketDescriptor)) {
    connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this,
            &TcpServer::onDisconeccted);
    qInfo() << "Подключение прошло успешно.";
  } else {
    qInfo() << "Ошибка :( ";
    delete socket;
  }
}

void TcpServer::onReadyRead() {
  QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
  if (!socket)
    return;

  QByteArray data = socket->readAll();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (doc.isNull() || !doc.isObject()) {
    qInfo() << parseError.errorString();
    return;
  }

  QJsonObject request = doc.object();
  QString command = request["command"].toString().toUpper();

  if (command == "PING")
    handlePing(socket);
  else if (command == "SUBSCRIBE")
    handleSubscribe(socket);
  else if (command == "UNSUBSCRIBE")
    handleUnsubscribe(socket);
  else if (command == "QUERY")
    handleQuery(socket, request);
}

void TcpServer::handlePing(QTcpSocket *socket) {
  QJsonObject response{{"response", "PONG"}};
  socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
}

void TcpServer::handleSubscribe(QTcpSocket *socket) {
  m_subscribers.insert(socket);
  QJsonObject response{{"response", "SUBSCRIBED"}};
  socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
}

void TcpServer::handleUnsubscribe(QTcpSocket *socket) {
  m_subscribers.remove(socket);
  QJsonObject response{{"response", "UNSUBSCRIBED"}};
  socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
}

void TcpServer::handleQuery(QTcpSocket *socket, const QJsonObject &request) {
  QString level = request["level"].toString();
  int n = request.contains("n") ? request["n"].toInt() : 10;

  QJsonObject response;
  response["command"] = "QUERY_RESULT";
  response["level"] = level;
  response["ema"] = m_engine.getEmaForLevel(level);
  response["p90"] = m_engine.calculatePercentile(level, 90.0);
  response["p95"] = m_engine.calculatePercentile(level, 95.0);
  response["p99"] = m_engine.calculatePercentile(level, 99.0);
  TrendLine trend = m_engine.getTrendForLevel(level);
  response["trend_b"] = trend.b;
  response["trend_k"] = trend.k;

  int forecastSteps = request.contains("forecast_steps")
                          ? request["forecast_steps"].toInt()
                          : 5;
  response["forecast"] = m_engine.predictForecast(level, forecastSteps);

  QJsonArray logsArray;
  QStringList recentLogs = m_engine.getLastNLogs(level, n);
  for (const QString &logStr : recentLogs) {
    logsArray.append(logStr);
  }
  response["logs"] = logsArray;

  socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
}

void TcpServer::onDisconeccted() {
  QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
  if (!socket)
    return;
  m_subscribers.remove(socket);
  qInfo() << "Клиент разорвал соединение";
  socket->deleteLater();
}