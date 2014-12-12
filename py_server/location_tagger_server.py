import tornado.ioloop
import tornado.web
import simplejson

from location_tagger import LocationTagger

HOSTNAME = "localhost"
PORT = 15555
DB_HOST = "hostname"
DB_USER = "user"
DB_PASSWORD = "password"
DB_NAME = "dbname"
DB_PORT = 3306

class MainHandler(tornado.web.RequestHandler):
    def get(self):
    	term = self.get_argument("q")
    	results = TAGGER.tag(term)
        self.write(simplejson.dumps(results))

application = tornado.web.Application([
    (r"/", MainHandler),
])

if __name__ == "__main__":
    application.listen(9999)

    global TAGGER
    TAGGER = LocationTagger(HOSTNAME, PORT, DB_HOST, DB_PORT, DB_USER, DB_PASSWORD, DB_NAME)
    tornado.ioloop.IOLoop.instance().start()
