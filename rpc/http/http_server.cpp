#include "rpc/http/http_server.h"
#include "rpc/http/http_context.h"
#include "rpc/http/http_response.h"

namespace rpc {
namespace http {

void defaultHttpCallback(const HttpRequest &,HttpResponse *resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventDispatcher *evd,const InetAddr &addr):
    _tcp_server(evd,addr),_http_callback(defaultHttpCallback) {
        _tcp_server.SetConnectCb(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
        _tcp_server.SetMessageCb(std::bind(&HttpServer::onMessage,this,std::placeholders::_1,std::placeholders::_2));
}
void HttpServer::onConnection(const ConnectionPtr &conn) {
    conn->SetContext(HttpContext());
}

void HttpServer::onMessage(const ConnectionPtr& conn,
                           IoBuffer* buf)
{
  HttpContext* context = std1::any_cast<HttpContext>(conn->GetContext());

  if (!context->parseRequest(buf))
  {
    std::string str("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Send(str.c_str(),str.size());
    conn->ForceClose();
  }

  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const ConnectionPtr& conn, const HttpRequest& req)
{
  const string& connection = req.getHeader("Connection");
  bool close = connection == "close" ||
    (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  _http_callback(req, &response);
  IoBuffer buf;
  response.appendToBuffer(&buf);
  conn->Send(buf.Peek(),buf.ReadableBytes());
  if (response.closeConnection())
  {
      conn->ForceClose();
  }
}


} // http
} // rpc
