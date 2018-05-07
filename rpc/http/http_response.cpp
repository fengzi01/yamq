// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "rpc/http/http_response.h"
#include <stdio.h>
#include "rpc/io_buffer.h"

namespace rpc {
namespace http {

void HttpResponse::appendToBuffer(IoBuffer* output) const
{
  char buf[32];
  size_t size = snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
  output->Append(buf,size);
  output->Append(statusMessage_);
  output->Append("\r\n");

  if (closeConnection_)
  {
    output->Append("Connection: close\r\n");
  }
  else
  {
    snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
    output->Append(buf);
    output->Append("Connection: Keep-Alive\r\n");
  }

  for (std::map<string, string>::const_iterator it = headers_.begin();
       it != headers_.end();
       ++it)
  {
    output->Append(it->first);
    output->Append(": ");
    output->Append(it->second);
    output->Append("\r\n");
  }

  output->Append("\r\n");
  output->Append(body_);
}
}
}
