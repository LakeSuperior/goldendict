#pragma once

#include "config.hh"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class EudicConnector: public QObject
{
  Q_OBJECT

public:
  explicit EudicConnector( QObject * parent, Config::Class const & cfg );

  void sendToEudic( QString const & word, QString text, QString const & sentence );

private:
  QNetworkAccessManager * mgr;
  Config::Class const & cfg;
  void postToEudic( QString const & postData );
  static constexpr auto transfer_timeout = 3000;

public:
signals:
  void errorText( QString const & );
private slots:
  void finishedSlot( QNetworkReply * reply );
};
