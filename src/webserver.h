/*
  This file is part of the PhantomJS project from Ofi Labs.

  Copyright (C) 2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Milian Wolff <milian.wolff@kdab.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QVariantMap>

///TODO: is this ok, or should it be put into .cpp
///      can be done by introducing a WebServerPrivate *d;
#include "mongoose.h"

class Config;

class WebServerRequest;
class WebServerResponse;

/**
 * Scriptable HTTP web server.
 *
 * see also: modules/webserver.js
 */
class WebServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString port READ port);

public:
    WebServer(QObject *parent, const Config *config);
    virtual ~WebServer();

public slots:
    /**
     * Start listening for incoming connections on port @p port.
     *
     * For each new request @c handleRequest() will be called which
     * in turn emits @c newRequest() where appropriate.
     *
     * @return true if we can listen on @p port, false otherwise.
     *
     * WARNING: must not be the same name as in the javascript api...
     */
    bool listenOnPort(const QString &port);
    /**
     * @return the port this server is listening on
     *         or an empty string if the server is closed.
     */
    QString port() const;

    /// Stop listening for incoming connections.
    void close();

signals:
    /// @p request is a WebServerRequest, @p response is a WebServerResponse
    void newRequest(QObject *request, QObject *response);

private slots:
    void handleRequest(mg_event event, mg_connection* conn, const mg_request_info* request,
                       bool* handled);

private:
    Config *m_config;
    mg_context *m_ctx;
    QString m_port;
};

/**
 * Incoming HTTP client request.
 */
class WebServerRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString method READ method)
    Q_PROPERTY(QString httpVersion READ httpVersion)
    Q_PROPERTY(int statusCode READ statusCode)
    Q_PROPERTY(bool isSSL READ isSSL)
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(QString queryString READ queryString)
    Q_PROPERTY(QString remoteIP READ remoteIP)
    Q_PROPERTY(int remotePort READ remotePort)
    Q_PROPERTY(QString remoteUser READ remoteUser)
    Q_PROPERTY(int headers READ headers)

public:
    WebServerRequest(const mg_request_info *request);

public slots:
    /// @return request method, i.e. Get/Post
    QString method() const;
    QString httpVersion() const;
    int statusCode() const;
    bool isSSL() const;
    QString url() const;
    QString queryString() const;
    QString remoteIP() const;
    int remotePort() const;
    QString remoteUser() const;

    //TODO: better javascript api that allows easy "for(i in headers)" iteration?
    //see e.g.: http://www.qtcentre.org/threads/31298-QtScript-bindings-to-advanced-containers-%28QMap-QList-etc%29
    int headers() const;
    QString headerName(int header) const;
    QString headerValue(int header) const;

private:
    const mg_request_info *m_request;
};

/**
 * Outgoing HTTP response to client.
 */
class WebServerResponse : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int statusCode READ statusCode WRITE setStatusCode);
    Q_PROPERTY(QVariantMap headers READ headers WRITE setHeaders);
public:
    WebServerResponse(mg_connection *conn);

public slots:
    /// send @p headers to client with status code @p statusCode
    void writeHeaders(int statusCode, const QVariantMap &headers);
    /// sends @p data to client and makes sure the headers are send beforehand
    void writeBody(const QString &data);

    /// get the currently set status code, 200 is the default
    int statusCode() const;
    /// set the status code to @p code
    void setStatusCode(int code);

    /// get the value of header @p name
    QString header(const QString &name) const;
    /// set the value of header @p name to @p value
    void setHeader(const QString &name, const QString &value);

    /// get all headers
    QVariantMap headers() const;
    /// set all headers
    void setHeaders(const QVariantMap &headers);

private:
    mg_connection *m_conn;
    int m_statusCode;
    QVariantMap m_headers;
    bool m_headersSent;
};

#endif // WEBSERVER_H
