/*******************************************************************************
*  file    : gsmmodem.cpp
*  created : 05.08.2013
*  author  : 
*******************************************************************************/

#include "gsmmodem.hpp"
#include "gsmsmsreader.hpp"
#include <qatchat.h>
#include <QTimer>
#include <QDebug>

GsmModem::GsmModem(QObject *parent) :
    QObject(parent),
    initialized_(false),
    initializing_(false),
    serial_baudrate_(9600),
    balanceCash_(0.)
{
    initGsmModem();
}

GsmModem::GsmModem(const QString &ussdCommandForRequestBalance, QObject *parent) :
    QObject(parent),
    initialized_(false),
    initializing_(false),
    serial_baudrate_(9600),
    balanceCash_(0.),
    ussdCommandForRequestBalance_(ussdCommandForRequestBalance)
{
    initGsmModem();
}

GsmModem::~GsmModem()
{
    delete at_chat_;
}

void GsmModem::requestBalance()
{
    if (!ussdCommandForRequestBalance_.isEmpty())
        chat(QString("AT+CUSD=1,%0,15").arg(ussdCommandForRequestBalance_));
}

double GsmModem::getBalanceCash()
{
    return balanceCash_;
}

void GsmModem::init(const QString& name, int baudrate)
{
    serial_baudrate_ = baudrate;
    if(initializing_ == false)
    {
        if(serial_->isOpen())
            serial_->close();
        if(!openSerial(name))
        {
            initialized_  = false;
            emit initialized(false);
            initializing_ = false;
            return;
        }

        QTimer::singleShot( 10000,this,SLOT(onModemInitTimeout()) );
        initializing_ = true;
        init_pos_ = 0;
        init_list_<<"AT"
                  <<"AT+CMEE=1" //??????
                  <<"AT+CMGF=0"// Sms PDU
                  <<"AT+CGMI"
                  <<"AT+CGMM"
                  <<"AT+CGMR"
                  <<"AT+CGSN"
                  <<"AT+CNMI=2,1,0,1,0"
                  <<"AT+CLIP=1"
                  <<"AT+COPS?"
                  <<"AT+CREG?";
        at_chat_->chat( "AT",this, SLOT(onModemInitResponce(bool,QAtResult)) );
    }
}

void GsmModem::onModemInitResponce(bool ok, const QAtResult& res)
{
    if(ok)
    {
        if(init_list_.at(init_pos_) == "AT+CGMI")
            manufacturer_ = res.content();
        else if(init_list_.at(init_pos_) == "AT+CGMM")
            model_ = res.content();
        else if(init_list_.at(init_pos_) == "AT+CGMR")
            revision_ = res.content();
        else if(init_list_.at(init_pos_) == "AT+CGSN")
            number_ = res.content();
        else if(init_list_.at(init_pos_) == "AT+COPS?") {
            operator_ = QString();
            QRegExp re("^\\+COPS:.*\"(.*)\"");
            if (re.indexIn(res.content(), 0) >= 0)
            {
                operator_ = re.cap(1);
                emit operatorName(operator_);
            }
        } else if (init_list_.at(init_pos_) == "AT+CREG?") {
            qDebug() << "at+creg";
            QRegExp re("^\\+CREG: *\\d+ *,\\s*(\\d+)\\s*$");
            if (re.indexIn(res.content(), 0) >= 0)
            {
                int status = re.cap(1).toInt();
                emit isRegisteredInNetwork(status);
                if (status == 1 || status == 5)
                    registeredInNetwork_ = true;
                else
                    registeredInNetwork_ = false;
            }
        }

        init_pos_++;
        if(init_pos_ >= init_list_.size())
        {
            initialized_ = true;
            emit initialized(true);
            initializing_ = false;
            if (!ussdCommandForRequestBalance_.isEmpty()) {
                requestBalance();
            }
            timer_->start(60000);
            onTimeoutForCheckSignalLevel();
        }
        else
        {
            at_chat_->chat(init_list_.at(init_pos_),this, SLOT(onModemInitResponce(bool,QAtResult)) );
        }
    }
    else
    {
        initialized_ = false;
        emit initialized(false);
        initializing_ = false;
        serial_->close();
    }
}

void GsmModem::onModemInitTimeout()
{
    if(initializing_ == true && initialized_ == false)
    {
        initialized_ = false;
        errorString_ = QString(tr("???????? ???????? ???????? ?????? ?? ??????."));
        emit initialized(false);
        initializing_ = false;
        serial_->close();
    }
}

void GsmModem::onNotification(const QString &command, const QString &value)
{
    if (command == QString("+CUSD:")) {
        // ballance?
        QRegExp re ("^\\s*(?:\\d+,\")([\\dA-F]+)(?:\",\\d+)");
        if (re.indexIn(value,0) >= 0)
        {
            QString text = re.cap(1);
            QByteArray a;
            bool ok;
            for (int i = 0; i < text.length() / 4; ++i)
            {
                a.append(text.mid(i*4 + 2, 2).toInt(&ok, 16));
                a.append(text.mid(i*4, 2).toInt(&ok, 16));
            }
            QString smsText = QString::fromUtf16((ushort*)a.data(), a.length() / 2);
            bool negative = false;
            // only for russian operators
            if (smsText.contains(tr("Минус")) || smsText.contains(tr("Отрицательный")))
                negative = true;
            QRegExp re("(?:w*)(-*\\d+[.,]\\d+)(?:.*)");
            if (re.indexIn(smsText,0) >= 0)
            {
                double temp = re.cap(1).toDouble();
                if (negative)
                    temp = temp * -1;
                balanceCash_ = temp;
                emit balanceCash(temp);
            }
        }
        re.setPattern("^\\s*\\d+,\"\\w+\\s*(\\d+[.,]\\d+).*");
        if (re.indexIn(value,0) >= 0)
        {
            balanceCash_ = re.cap(1).toDouble();
            emit balanceCash(balanceCash_);
        }
    } else if (command == "+CSQ:") {
        qDebug() << value;
        QRegExp re ("^\\s*(\\d+)\\s*,\\s*(\\d+)");
            if (re.indexIn(value,0) >= 0)
            {
                QString signalLevel = re.cap(1);
                QString errorLevel = re.cap(2);
                signalLevel_ = signalLevel.toInt();
                errorLevel_ = errorLevel.toInt();
                emit signalLevels(signalLevel_, errorLevel_);
            }
    }
}

void GsmModem::onTimeoutForCheckSignalLevel()
{
    if (isRegisteredInNetwork() && primaryAtChat()) {
        primaryAtChat()->chat("AT+CSQ");
    }
}
QString GsmModem::number() const
{
    return number_;
}

QString GsmModem::operatorName() const
{
    return operator_;
}

bool GsmModem::isRegisteredInNetwork() const
{
    return registeredInNetwork_;
}

QString GsmModem::errorString() const
{
    return errorString_;
}

bool GsmModem::isValid() const
{
    return initialized_;
}

void GsmModem::initGsmModem()
{
    serial_ = new QSerialPort(this);
    timer_  = new QTimer     (this);

    at_chat_ = new QAtChat(serial_);
    connect(at_chat_, SIGNAL(notification(QString,QString)), this, SLOT(onNotification(QString,QString)));
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimeoutForCheckSignalLevel()));
}

QString GsmModem::revision() const
{
    return revision_;
}

QString GsmModem::model() const
{
    return model_;
}

QString GsmModem::manufacturer() const
{
    return manufacturer_;
}

bool GsmModem::openSerial(const QString& name)
{
    bool res = false;
    errorString_ = "";
    serial_->setPortName(name);
    if (serial_->open(QIODevice::ReadWrite))
    {
        if (       serial_->setBaudRate   (serial_baudrate_)
                   && serial_->setDataBits   (QSerialPort::Data8)
                   && serial_->setParity     (QSerialPort::NoParity)
                   && serial_->setStopBits   (QSerialPort::OneStop)
                   && serial_->setFlowControl(QSerialPort::NoFlowControl)
                   )
        {
            res = true;
        }
        else
        {
            serial_->close();
            errorString_ = QString("Can't configure the serial port: %1, "
                                   "error code: %2. Error text: %3")
                    .arg(name).arg(serial_->error()).arg(serial_->errorString());
        }
    }
    else
    {
        errorString_ = QString("Can't opened the serial port: %1, "
                               "error code: %2. Error text: %3")
                .arg(name).arg(serial_->error()).arg(serial_->errorString());
    }
    return res;
}

QAtChat*GsmModem::primaryAtChat()
{
    return at_chat_;
}

/*!
    Sends \a command to the modem on the primary AT chat channel.
    If the command fails, the caller will not be notified.

    \sa primaryAtChat(), retryChat()
*/

void GsmModem::chat(const QString &command)
{
    at_chat_->chat( command );
}

/*!
    Sends \a command to the modem on the primary AT chat channel.
    When the command finishes, notify \a slot on \a target.  The slot
    has the signature \c{done(bool,QAtResult&)}.  The boolean parameter
    indicates if the command succeeded or not, and the QAtResult parameter
    contains the full result data.

    The optional \a data parameter can be used to pass extra user data
    that will be made available to the target slot in the QAtResult::userData()
    field.

    \sa primaryAtChat(), retryChat()
*/
void GsmModem::chat(const QString &command, QObject *target, const char *slot, QAtResult::UserData *data)
{
    at_chat_->chat( command, target, slot, data );
}
