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

import logging
 
import pygame
import time
import json
import random
 
# @@@ improve naming: extBroadcast, Chat, Index.
# Serve more than just index.

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
        cls.send_all_raw(json.dumps({'type': 'serverMsg', 'data': msg}))
    
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
        # Add client to the clients list
        self.participants.add(self)
        print("there is a new client")
        print(len(self.participants))
        # Tell everyone that someone joined
        self.send_all_json({"type":"serverMsg","data":"Someone joined!"})
        
        if len(self.participants) == 1:
            plural = ""
        else:
            plural = "s"
        
        if len(self.participants) > 1:
            comments = ["-u got frineds","-YaY!","-uthr hoomans"] # List of comments for there being other users
            comment = random.SystemRandom().choice(comments) # randomly selects a "comment" to add
        else:
            comments = ["-aww... so sad :(","-#ForeverAlone","-all by yourself..."] # List of comments on there being 0 users
            comment = random.SystemRandom().choice(comments) # Selects a random comment
        
        self.playerData = {"name":"noname","colour":"lime"}
        self.send_all_json({"type":"data","data":{"numUsers":len(self.participants),"numUsersComment":comment,"numUsersPlural":plural}})

    def on_message(self, message):
        """Process a message received from this client."""
        logging.getLogger().info("Received %s", message)

        try:
            obj = json.loads(message)
            typ = obj["type"]
        except Exception as e:
            # Something wrong with the message - complain to sender only.
            self.send(json.dumps({'type': 'serverMsg', "data":"Bad message received: " + str(e)}))
            return

        if typ == 'chat':
            self.send_all_json(obj) # Bounces the same message to everyone
            self.playerData = obj["data"]["sender"] # Modifies the playerData
            print(self.playerData)
            
            '''elif typ == 'cmd' and obj["cmd"] == "quit":
            # Handle quit command by posting a QUIT event to the Pygame event loop.
            pygame.event.post(pygame.event.Event(pygame.QUIT))'''
            
        elif typ == 'input':
            data.handleUpData(obj["data"])
        else:
            pass # Ignore other types of messages for now.
    
    def on_close(self):
        """Process this client leaving."""
        # Remove client from the clients list
        self.participants.remove(self)
        
        # Broadcasts how many users are left
        if len(self.participants) == 1: # If there are no other users - add a comment
            numCommentsList = ["aww... so sad :(","#ForeverAlone","all by yourself..."] # List of comments on there being 0 users
            numComment = random.SystemRandom().choice(numCommentsList) # Selects a random comment
            self.send_all_json({"type":"data","data":{"numUsers":len(self.participants),"numUsersComment":numComment,"numUsersPlural":""}})
            print("Some hooman quitificationdid. There be only 1 survivior.") 
        else: # Otherwise just send the number of users
            self.send_all_json({"type":"data","data":{"numUsers":len(self.participants),"numUsersPlural":"s"}})
        
        # Tell everyone that someone left
        opinionList = ["... what a shame :(","! How DARE they!","... freeEEDDDOOOOMM!!!",". Or have they?!?"] # List of random "opinions"
        opinion = random.SystemRandom().choice(opinionList) # Selects a random "opinion" to add
        leaveMsg = "<span color='"+ self.playerData["colour"] +"'>"+ self.playerData["name"] +"</span> has quit"+ opinion
        self.send_all_json({"type":"serverMsg","data":leaveMsg})

class IndexHandler(tornado.web.RequestHandler):
    """Regular HTTP handler to serve the main page"""
    def get(self):
        self.render('FlightStatus.html')

def startChatConnection():
    print("Initialising ChatConnection.")
    # Set up logging
    logging.getLogger().setLevel(logging.DEBUG)
    
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
    
    # Make Tornado app listen on port 8080
    port = 8080
    app.listen(port)
    print("Listening on HTTP port %d" % port)
    print("Use the arrow keys to control; press Ctrl+C to exit.")
    
    from mainGroundControl import mainLoop
    
    tornado.ioloop.PeriodicCallback(mainLoop, 10).start()
    
    # Start the IOLoop. This only returns when the application shuts down.
    tornado.ioloop.IOLoop.instance().start()

if __name__ == "__main__": # If this is being run standalone
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
    
    # Make Tornado app listen on port 8080
    port = 8080
    app.listen(port)
    print("Listening on HTTP port %d" % port)
    print("Use the arrow keys to control; press Ctrl+C to exit.")
    
    # Start the IOLoop. This only returns when the application shuts down.
    tornado.ioloop.IOLoop.instance().start()
