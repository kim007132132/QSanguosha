#include "nativesocket.h"
#include "settings.h"

#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>


NativeServerSocket::NativeServerSocket()
{
    server = new QTcpServer(this);
}

bool NativeServerSocket::listen(){
    return server->listen(QHostAddress::Any, Config.Port);
}

NativeClientSocket::NativeClientSocket()
    :socket(new QTcpSocket(this))
{
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

NativeClientSocket::NativeClientSocket(QTcpSocket *socket)
    :socket(socket)
{
    socket->setParent(this);
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

void NativeClientSocket::connectToHost(){
    socket->connectToHost(Config.HostAddress, Config.Port);
    connect(socket, SIGNAL(readyRead()), this, SLOT(emitReplies()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(raiseError(QAbstractSocket::SocketError)));
}

typedef char buffer_t[1024];

void NativeClientSocket::emitReplies(){
    while(socket->canReadLine()){
        buffer_t reply;
        socket->readLine(reply, sizeof(reply));

        emit reply_got(reply);
    }
}

void NativeClientSocket::disconnectFromHost(){    
    socket->disconnectFromHost();
}

void NativeClientSocket::send(const QString &message){
    socket->write(message.toAscii());
    socket->write("\n");
}

bool NativeClientSocket::isConnected() const{
    return socket->state() == QTcpSocket::ConnectedState;
}


QString NativeClientSocket::peerName() const{
    return socket->peerName();
}

void NativeClientSocket::raiseError(QAbstractSocket::SocketError socket_error){
    // translate error message
    QString reason;
    switch(socket_error){
    case QAbstractSocket::ConnectionRefusedError:
        reason = tr("Connection was refused or timeout"); break;
    case QAbstractSocket::RemoteHostClosedError:
        reason = tr("Remote host close this connection"); break;
    case QAbstractSocket::HostNotFoundError:
        reason = tr("Host not found"); break;
    case QAbstractSocket::SocketAccessError:
        reason = tr("Socket access error"); break;
    case QAbstractSocket::NetworkError:
        reason = tr("Server's' firewall blocked the connection or the network cable was plugged out"); break;
        // FIXME
    default: reason = tr("Unknow error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}