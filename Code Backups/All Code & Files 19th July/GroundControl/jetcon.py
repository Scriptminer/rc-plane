class Jetcon:
  def __init__(self):
    self.x = 0
    self.y = 0

  def display(self):
    print("Status: X:%+6.3f, Y:%+6.3f" % (self.x, self.y))
    ChatConnection.send_all_json({'type': 'pos', 'x': self.x, 'y': self.y })

  def tick(self):
    changed = False
    pygame.event.pump
    for event in pygame.event.get():

      if event.type == pygame.QUIT:
        print("Bye!")
        ChatConnection.send_all_msg("Server quitting")
        ioloop = tornado.ioloop.IOLoop.instance()
        ioloop.add_callback(ioloop.stop)


    if changed:
      self.display()
