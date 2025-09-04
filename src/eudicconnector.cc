#include "eudicconnector.hh"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "utils.hh"

QString markTargetWord2( QString const & sentence, QString const & word )
{
  // TODO properly handle inflected words.
  QString result = sentence;
  return result.replace( word, "<b>" + word + "</b>", Qt::CaseInsensitive );
}

EudicConnector::EudicConnector( QObject * parent, Config::Class const & _cfg ):
  QObject{ parent },
  cfg( _cfg )
{
  mgr = new QNetworkAccessManager( this );
  connect( mgr, &QNetworkAccessManager::finished, this, &EudicConnector::finishedSlot );
}

void EudicConnector::sendToEudic( QString const & word, QString text, QString const & sentence )
{
  if ( word.isEmpty() ) {
    emit this->errorText( tr( "eudic: can't create a card without a word" ) );
    return;
  }

  // Anki doesn't understand the newline character, so it should be escaped.
  text = text.replace( "\n", "<br>" );

  QString const postTemplate = R"anki({
      "action": "addNote",
      "version": 6,
      "params": {
          "note": {
              "deckName": "%1",
              "modelName": "%2",
              "fields": %3,
              "options": {
                  "allowDuplicate": true
              },
              "tags": []
          }
      }
  })anki";
  QJsonObject fields;
  fields.insert( cfg.preferences.ankiConnectServer.word, word );
  fields.insert( cfg.preferences.ankiConnectServer.text, text );
  if ( !cfg.preferences.ankiConnectServer.sentence.isEmpty() ) {
    QString sentence_changed = markTargetWord2( sentence, word );
    fields.insert( cfg.preferences.ankiConnectServer.sentence, sentence_changed );
  }

  QString postData = postTemplate.arg( cfg.preferences.ankiConnectServer.deck,
                                       cfg.preferences.ankiConnectServer.model,
                                       Utils::json2String( fields ) );

  QString const postTemplate2 = R"({
    "id": "0",
    "language": "en",
    "words": [ "%1" ]
   })";
  QJsonObject fields2;
  fields2.insert( "words", word );
   QString postData2 = postTemplate2.arg( word );
  //  qDebug().noquote() << postData;
  postToEudic( postData2 );
}

void EudicConnector::postToEudic( QString const & postData )
{
  QUrl url( "https://api.frdic.com/api/open/v1/studylist/words" );
  //url.setScheme( "https" );
  //url.setUrl( "https://api.frdic.com/api/open/v1/studylist/words" );
  //url.setHost( cfg.preferences.ankiConnectServer.host );
  //url.setPort( cfg.preferences.ankiConnectServer.port );
  //url.setAuthority( )
  QNetworkRequest request( url );
  request.setTransferTimeout( transfer_timeout );
  //  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  
  // ÉèÖÃ Authorization Í·
  QString tokenHeader = /*"Bearer " + */"NIS 7xnJAcOT5aXdWzE6khQtj+G0bkpFYmJtbGkHiOwAdq6iJLkuh7VPdA==";
  request.setRawHeader( "Authorization", tokenHeader.toUtf8() );

  auto reply = mgr->post( request, postData.toUtf8() );
  connect( reply, &QNetworkReply::errorOccurred, this, [ this ]( QNetworkReply::NetworkError e ) {
    qWarning() << e;
    emit this->errorText( tr( "eudic: post to eudic failed" ) );
  } );
}

void EudicConnector::finishedSlot( QNetworkReply * reply )
{
  if ( reply->error() == QNetworkReply::NoError ) {
    QByteArray const bytes   = reply->readAll();
    QJsonDocument const json = QJsonDocument::fromJson( bytes );
    auto const obj           = json.object();

    // Normally AnkiConnect always returns result and error,
    // unless Anki is not running.
    //if ( obj.size() == 2 && obj.contains( "result" ) && obj.contains( "error" ) && obj[ "error" ].isNull() ) {
    //  emit errorText( tr( "eudic: post to eudic success" ) );
    //}
    //else {
    //  emit errorText( tr( "eudic: post to eudic failed" ) );
    //}

    qDebug().noquote() << "eudic response:" << Utils::json2String( obj );
  }
  else {
    qDebug() << "eudic connect error" << reply->errorString();
    emit errorText( "eudic:" + reply->errorString() );
  }

  reply->deleteLater();
}
