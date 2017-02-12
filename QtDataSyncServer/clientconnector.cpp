#include "clientconnector.h"
#include <QFile>
#include <QSslKey>
#include <QWebSocket>
#include "app.h"

ClientConnector::ClientConnector(QObject *parent) :
	QObject(parent),
	server(nullptr),
	clients()
{
	auto name = qApp->configuration()->value("server/name", QCoreApplication::applicationName()).toString();
	auto mode = qApp->configuration()->value("server/wss", false).toBool() ? QWebSocketServer::SecureMode : QWebSocketServer::NonSecureMode;

	server = new QWebSocketServer(name, mode, this);
	connect(server, &QWebSocketServer::newConnection,
			this, &ClientConnector::newConnection);
	connect(server, &QWebSocketServer::serverError,
			this, &ClientConnector::serverError);
	connect(server, &QWebSocketServer::sslErrors,
			this, &ClientConnector::sslErrors);
}

bool ClientConnector::setupWss()
{
	if(server->secureMode() != QWebSocketServer::SecureMode)
		return true;

	auto filePath = qApp->configuration()->value("server/wss/pfx").toString();
	filePath = qApp->absolutePath(filePath);

	QSslKey privateKey;
	QSslCertificate localCert;
	QList<QSslCertificate> caCerts;

	QFile file(filePath);
	if(!file.open(QIODevice::ReadOnly))
		return false;

	if(QSslCertificate::importPkcs12(&file,
								  &privateKey,
								  &localCert,
								  &caCerts,
								  qApp->configuration()->value("server/wss/pass").toString().toUtf8())) {
		auto conf = server->sslConfiguration();
		conf.setLocalCertificate(localCert);
		conf.setPrivateKey(privateKey);
		caCerts.append(conf.caCertificates());
		conf.setCaCertificates(caCerts);
		server->setSslConfiguration(conf);
		return true;
	} else
		return false;
}

bool ClientConnector::listen()
{
	auto host = qApp->configuration()->value("server/host", QHostAddress(QHostAddress::Any).toString()).toString();
	auto port = (quint16)qApp->configuration()->value("server/port", 0).toUInt();
	if(server->listen(QHostAddress(host), port)) {
		qInfo() << "Listening on port" << server->serverPort();
		return true;
	} else {
		qCritical() << "Failed to create server with error:"
					<< server->errorString();
		return false;
	}
}

void ClientConnector::newConnection()
{
	while (server->hasPendingConnections()) {
		auto socket = server->nextPendingConnection();
		auto client = new Client(socket, this);
		connect(client, &Client::connected, this, [=](QUuid devId){
			clients.insert(devId, client);
			qDebug() << "connected" << devId;
			connect(client, &Client::destroyed, this, [=](){
				clients.remove(devId);
				qDebug() << "disconnected" << devId;
			});
		});
	}
}

void ClientConnector::serverError()
{
	qWarning() << "Server error"
			   << server->errorString();
}

void ClientConnector::sslErrors(const QList<QSslError> &errors)
{
	foreach(auto error, errors) {
		qWarning() << "SSL errors"
				   << error.errorString();
	}
}