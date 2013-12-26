/*******************************************************************************
*  file    : gsmmodem.hpp
*  created : 05.08.2013
*  author  : 
*******************************************************************************/

#ifndef GSMMODEM_HPP
#define GSMMODEM_HPP

#include <QObject>
#include <QSerialPort>
#include <QQueue>
#include <QStringList>
#include <qatresult.h>

class QAtChat;
class QTimer;
class GsmSmsReader;

class GsmModem : public QObject
{
    Q_OBJECT
public:
    explicit GsmModem(QObject *parent = 0);
    explicit GsmModem(const QString &ussdCommandForRequestBalance_, QObject *parent);
    ~GsmModem();
    void requestBalance();
    double getBalanceCash();
    void setBalanceCash(const double &cash) {balanceCash_ = cash;}

    virtual void init    (const QString &name, int baudrate);
    bool     openSerial  (const QString &name);
    QAtChat* primaryAtChat();
    void     chat( const QString& command );
    void     chat( const QString& command, QObject *target, const char *slot,
                   QAtResult::UserData *data = 0 );

    QString manufacturer() const;
    QString model       () const;
    QString revision    () const;
    QString number      () const;
    QString operatorName() const;
    bool isRegisteredInNetwork() const;

    QString errorString () const;
    bool    isValid     () const;

private:
    void initGsmModem();

private slots:
    void     onModemInitResponce(bool, const QAtResult& res);
    void     onModemInitTimeout();
    void     onNotification(const QString &command, const QString &value);
    void     onTimeoutForCheckSignalLevel();
signals:
    void initialized(bool);
    void balanceCash(double);
    void operatorName(QString operatorName);
    void isRegisteredInNetwork(int registered);
    void signalLevels(int rssi, int bor);

protected:
    bool initialized_;
    bool initializing_;
private:
    QStringList                 init_list_;
    int                         init_pos_;
    QSerialPort*                serial_ ;
    QAtChat*                    at_chat_;
    int                         serial_baudrate_;
    double                      balanceCash_;
    QString                     ussdCommandForRequestBalance_;

    QString                     errorString_;
    QTimer*                     timer_;

private:
    QString                     manufacturer_;
    QString                     model_;
    QString                     revision_;
    QString                     number_;
    QString                     operator_;
    int                         signalLevel_;
    int                         errorLevel_;
    bool                        registeredInNetwork_ = false;

};


#endif // GSMMODEM_HPP
