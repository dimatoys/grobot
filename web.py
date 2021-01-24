import sys
import io

from flask import Flask, Response, send_file
import json

from hardware import init

robot = init()
app = Flask(__name__, static_folder='html', static_url_path="/html")

@app.route('/')
def index():
	return app.send_static_file('index.html')

@app.route('/readSensors')
def readSensors():
	return robot.readSensors()

@app.route('/getServos')
def getServos():
	return json.dumps(robot.getServos())

@app.route('/setServoValue/<servo>/<value>')
def setServoValue(servo, value):
	robot.setServoValue(servo, value)
	return "OK"

@app.route('/setServoAngle/<servo>/<angle>')
def setServoAngle(servo, angle):
	robot.setServoAngle(servo, angle)
	return json.dumps(robot.getServos())

@app.route('/setLowMode/<value>')
def setLowMode(value):
	robot.setLowMode(value)
	return json.dumps(robot.getServos())

@app.route('/depth')
def depth():
	jpg = robot.showDepth()
	return send_file(io.BytesIO(jpg),mimetype='image/jpeg')

@app.route('/scan')
def scan():
	jpg = robot.scan()
	return send_file(io.BytesIO(jpg),mimetype='image/jpeg')

# sudo FLASK_APP=web.py /home/pi/.local/bin/flask run
