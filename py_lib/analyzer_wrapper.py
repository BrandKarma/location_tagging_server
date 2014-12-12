# coding=utf-8
import struct
import socket
import re
import exceptions
import urllib
import urllib2
import simplejson

INT_SIZE = 4
FLOAT_SIZE = 4
BYTE_SIZE = 1

class AnalyzerException(exceptions.Exception):
    def __init__(self, code, description):
        self.code = code
        self.description = description
        return

    def __str__(self):
        print "[%d] %s" % (self.code, self.description)


class Marshaler:
    def __init__(self):
        self.data = []
        self.size = INT_SIZE

    def add_integer(self, i):
        #print "add_integer(): %d" % i
        self.data.append(struct.pack("!i", i))
        self.size += INT_SIZE

    def add_string(self, s):
        bytes = s.encode('utf-8')
        self.add_integer(len(bytes))
        #print "add_string(): %s [len: %d]" % (s, len(bytes))
        self.data.append(bytes)
        self.size += len(bytes)

    def add_byte(self, b):
        #print "add_byte(): %c" % b
        self.data.append(struct.pack("!b", b))
        self.size += BYTE_SIZE

    def add_float(self, f):
        #print "add_float(): %f" % f
        self.data.append(struct.pack("!f", f))
        self.size += FLOAT_SIZE

    def get_bytes(self):
        #print "total_size: %d" % self.size
        size = struct.pack("!i", self.size)
        self.data.insert(0, size)
        return "".join(self.data)


class Unmarshaler:
    def get_integer(self):
        i = struct.unpack_from("!i", self.data, self.cursor)
        self.cursor += INT_SIZE
        #print "get_integer(): %d" % i[0]
        return i[0]

    def __init__(self, data):
        self.data = data
        self.cursor = 0
        reported_size = self.get_integer()
        if reported_size != len(data):
            raise AnalyzerException(1, "While unmarshaling data, got %d bytes, expected %d" % (len(data), reported_size))

    def get_float(self):
        f = struct.unpack_from("!f", self.data, self.cursor)
        self.cursor += FLOAT_SIZE
        #print "get_float(): %f" % f[0]
        return f[0]

    def get_boolean(self):
        b = struct.unpack_from("B", self.data, self.cursor)
        self.cursor += BYTE_SIZE
        return b[0] != 0


class AnalyzerInput:
    def __init__(self,
                 text):
        self.text = text

    def get_bytes(self):
        m = Marshaler()
        m.add_integer(VERSION)
        m.add_string(self.text)
        return m.get_bytes();


class AnalyzerOutput:
    def __init__(self, data, input):
        self.input = input
        u = Unmarshaler(data)

        self.regions = []
        num_regions = u.get_integer()
        for i in range(num_sentences):
            self.regions.append(u.get_string())


CHUNK_SIZE=4096
VERSION=2

class AnalyzerClient:
    def __init__(self, hostname, port):
        self.hostname = hostname
        self.port = port

    def analyze(self, s):
        cleaned = strip_html(s)
        input_obj = AnalyzerInput(cleaned)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            sock.connect((self.hostname, self.port))

            outgoing_data = input_obj.get_bytes()
            sock.send(outgoing_data)

            received_data = []
            chunk = sock.recv(CHUNK_SIZE)
            while len(chunk) > 0:
                received_data.append(chunk)
                chunk = sock.recv(CHUNK_SIZE)
        except socket.error:
            raise AnalyzerException(2, "Unable to connect to analyzer")

        data = "".join(received_data)
        output = AnalyzerOutput(data, input_obj)

        return {"input":cleaned, "regions":output.regions}


def strip_html(html):
    temp = html.strip()
    temp = re.sub("&\\w+;"," ", temp)
    temp = re.sub("\\\\r|\\r","", temp)
    return temp.replace("...", "... ")


def test_analyze(s, hostname="localhost", port=15001):
    c = AnalyzerClient(hostname, port)
    return c.analyze(s)

if __name__=="__main__":
    import sys
    s = sys.argv[1]
    print test_analyze(s)
