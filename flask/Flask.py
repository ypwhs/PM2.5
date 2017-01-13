# coding=utf-8
import sqlite3
from flask import Flask, request, session, g, redirect, url_for, abort, render_template, flash
import datetime
import os
import time

app = Flask(__name__)
app.config['DATABASE'] = 'pm.db'

def get_db():
    if not hasattr(g, '_database'):
        db = sqlite3.connect(app.config['DATABASE'])

        def make_dicts(cursor, row):
            return dict((cursor.description[idx][0], value)
                        for idx, value in enumerate(row))

        db.row_factory = make_dicts
        g._database = db
    return g._database


@app.teardown_appcontext
def close_db(error):
    if hasattr(g, '_database'):
        g._database.close()


def query_db(query, args=(), one=False):
    cur = get_db().execute(query, args)
    rv = cur.fetchall()
    cur.close()
    return (rv[0] if rv else None) if one else rv
    return rv


@app.route('/')
def index():
    return redirect(url_for('index_hour', hour=8))


@app.route('/<int:hour>')
def index_int(hour):
    return redirect(url_for('index_hour', hour=hour))


@app.route('/<float:hour>')
def index_float(hour):
    return redirect(url_for('index_hour', hour=hour))


@app.route('/<float:hour>hour')
def index_hour(hour):
    before = int(time.time()*1000) - hour*3600*1000
    data = query_db('select * from pm where timestamp>%d' % before)
    while len(data) > 10000:
        data = data[::2]
    times = map(lambda x:x['timestamp'], data)
    pm1_0 = map(lambda x:x['pm1_0'], data)
    pm2_5 = map(lambda x:x['pm2_5'], data)
    pm10 = map(lambda x:x['pm10'], data)
    co2 = map(lambda x:x['co2'], data)
    return render_template('chart.html', date=times, pm1_0=pm1_0, pm2_5=pm2_5, pm10=pm10, co2=co2)

@app.errorhandler(404)
def page_not_found(error):
    return render_template('404.html'), 404

if __name__ == '__main__':
    app.run(debug=True)
