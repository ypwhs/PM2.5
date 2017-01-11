import socket
import sqlite3

database = 'pm.db'
UDP_IP = "0.0.0.0"
UDP_PORT = 20160

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

db = sqlite3.connect(database)
def make_dicts(cursor, row):
    return dict((cursor.description[idx][0], value)
                for idx, value in enumerate(row))
db.row_factory = make_dicts

while True:
    data, addr = sock.recvfrom(1024)
    data = data.split(',')
    if len(data) != 5:
        continue
    data = map(int, data)
    print data
    timestamp, pm1_0, pm2_5, pm10, co2 = data
    db.execute('INSERT INTO pm (timestamp, pm1_0, pm2_5, pm10, co2) VALUES'
    ' (?, ?, ?, ?, ?)', (timestamp, pm1_0, pm2_5, pm10, co2))
    db.commit()
