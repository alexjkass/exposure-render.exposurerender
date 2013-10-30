
#include "basesocket.h"

QBaseSocket::QBaseSocket(QObject* Parent /*= 0*/) :
	QTcpSocket(Parent),
	BlockSize(0)
{
	connect(this, SIGNAL(readyRead()), this, SLOT(OnReadyRead()), Qt::DirectConnection);
}

QBaseSocket::~QBaseSocket()
{
}

void QBaseSocket::OnReadyRead()
{
	QDataStream DataStream(this);

    DataStream.setVersion(QDataStream::Qt_4_0);

	if (this->bytesAvailable() < this->BlockSize)
		return;

	while (this->bytesAvailable() >=sizeof(quint32))
	{
		if (this->BlockSize == 0)
			DataStream >> this->BlockSize;

		if (this->bytesAvailable() < this->BlockSize)
			return;

		QString Action;

		DataStream >> Action;

		this->OnReceiveData(Action, DataStream);

		this->BlockSize = 0;
	}
}

void QBaseSocket::OnReceiveData(const QString& Action, QDataStream& DataStream)
{
	qDebug() << "Not implemented";
}

void QBaseSocket::SendData(const QString& Action, QByteArray Data)
{
	qDebug() << "Sending" << Action << " of " << Data.count() << "bytes";

	QByteArray ByteArray;

	QDataStream DataStream(&ByteArray, QIODevice::WriteOnly);
	DataStream.setVersion(QDataStream::Qt_4_0);

	DataStream << quint32(0);

	DataStream << Action;
	DataStream << Data;

	DataStream.device()->seek(0);
		    
	DataStream << (quint32)(ByteArray.size() - sizeof(quint32));

    this->write(ByteArray);
	this->flush();
}