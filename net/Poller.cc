// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/coserv/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "coserv/net/Poller.h"

#include "coserv/net/Channel.h"

using namespace coserv;
using namespace coserv::net;

Poller::Poller(EventLoop* loop)
  : ownerLoop_(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
  assertInLoopThread();
  ChannelMap::const_iterator it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  return new EPollPoller(loop);
}