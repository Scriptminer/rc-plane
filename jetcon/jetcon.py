#!/bin/env python
# -*- coding: utf-8 -*-
"""
    Server script for connecting pygame to a sockjs client using
    sockjs-tornado.  Based on sockjs-tornado chat example.

    Listens on port 8080.

    See https://github.com/mrjoes/sockjs-tornado for further details.
    Docs for sockjs-tornado are at http://sockjs-tornado.readthedocs.io/en/latest/index.html
"""

import tornado.ioloop
import tornado.web
import sockjs.tornado
import os

import pygame
import time
import json

# @@@ improve naming: extBroadcast, Chat, Index.
# Serve more than just index.

class Jetcon:
  def __init__(self):
    self.x = 0
    self.y = 0

  def display(self):
    print("Status: X:%+6.3f, Y:%+6.3f" % (self.x, self.y))
    ChatConnection.send_all_json({'type': 'pos', 'x': self.x, 'y': self.y })

  def tick(self):
    changed = False
    for event in pygame.event.get():

      if event.type == pygame.QUIT:
        print("Bye!")
        ChatConnection.send_all_msg("Server quitting")
        ioloop = tornado.ioloop.IOLoop.instance()
        ioloop.add_callback(ioloop.stop)

      elif event.type == pygame.KEYDOWN:
        if event.key == pygame.K_UP:
          self.y += 0.01
          changed = True
        elif event.key == pygame.K_DOWN:
          self.y -= 0.01
          changed = True
        elif event.key == pygame.K_LEFT:
          self.x -= 0.01
          changed = True
        elif event.key == pygame.K_RIGHT:
          self.x += 0.01
          changed = True

    if changed:
      self.display()

class IndexHandler(tornado.web.RequestHandler):
    """Regular HTTP handler to serve the main page"""
    def get(self):
        self.render('index.html')


class ChatConnection(sockjs.tornado.SockJSConnection):
    """Chat connection implementation"""
    # Class level variable
    participants = set()

    @classmethod
    def send_all_json (cls, msg):
      """Send JSON blob to all instances of this class."""
      cls.send_all_raw(json.dumps(msg))

    @classmethod
    def send_all_msg(cls, msg):
      """Send message (wrapped in JSON) to all instances of this class."""
      cls.send_all_raw(json.dumps({'type': 'msg', 'msg': msg}))

    @classmethod
    def send_all_raw(cls, msg):
      """Send message to all instances of this class."""
      # Clever trick to pick just one element of the set and use it.
      # Does nothing if there are no participants to send to.
      for p in cls.participants:
        p.broadcast(cls.participants, msg)
        break

    def on_open(self, info):
      """Process this new client joining."""
      # Tell everyone that someone joined
      self.send_all_msg("Someone joined.")

      # Add client to the clients list
      self.participants.add(self)

    def on_message(self, message):
      """Process a message received from this client."""
      logging.getLogger().info("Received %s", message)

      try:
        obj = json.loads(message)
        typ = obj["type"]
      except Exception as e:
        # Something wrong with the message - complain to sender only.
        self.send(json.dumps({'type': 'msg', 'msg': "Bad message received: " + str(e)}))
        return

      if typ == 'msg':
        # Broadcast message to everyone else
        self.send_all_msg(obj["msg"])
      elif typ == 'cmd' and obj["cmd"] == "quit":
        # Handle quit command by posting a QUIT event to the Pygame event loop.
        pygame.event.post(pygame.event.Event(pygame.QUIT))
      else:
        # Ignore other types of messages for now.
        pass

    def on_close(self):
      """Process this client leaving."""
      # Remove client from the clients list
      self.participants.remove(self)

      # Tell everyone that someone left
      self.send_all_msg("Someone left.")

if __name__ == "__main__":
  # Set up logging
  import logging
  logging.getLogger().setLevel(logging.DEBUG)

  # Initialise pygame
  pygame.init()
  pygame.display.set_mode((100,100))

  # Create main routers.
  staticPath = os.path.join(os.path.dirname(__file__), "static")
  mainRoutes = [
    (r"/", IndexHandler),
    (r"/static/(.*)", tornado.web.StaticFileHandler, {"path": staticPath})
  ]

  # Create chat router.
  chatRouter = sockjs.tornado.SockJSRouter(ChatConnection, '/chat')

  # Create Tornado web application
  app = tornado.web.Application(
          mainRoutes + chatRouter.urls,
          debug = True
  )

  # 3. Make Tornado app listen on port 8080
  port = 8080
  app.listen(port)
  print("Listening on HTTP port %d" % port)
  print("Use the arrow keys to control; press Ctrl+C to exit.")

  # Set up Jetcon and schedule its main loop (including pygame events)
  # every few milliseconds.
  jetcon = Jetcon()
  tornado.ioloop.PeriodicCallback(jetcon.tick, 10).start()

  # Start the IOLoop. This only returns when the application shuts down.
  tornado.ioloop.IOLoop.instance().start()
