from ctypes import *

g_GRobotModule = None

g_Servos = ["L", "G", "A", "B"]
g_ServoMap = {}


class TServo(Structure):
	_fields_ = [("angle", c_int),
	           ("hardware_min", c_int),
	           ("hardware_max", c_int)]

class TPicture(Structure):
	_fields_ = [("buffer_size", c_int,),
	           ("buffer", POINTER(c_char))]

	def getJPG(self):
		return self.buffer[:self.buffer_size]

class TGRobot(Structure):
	_fields_ = [("pca9685", c_void_p),
	           ("camera", c_void_p),
	           ("numServos", c_int),
	           ("servos", TServo * 16),
	           ("picture", TPicture)]

	def __init__(self):
		global g_GRobotModule
		g_GRobotModule.init(byref(self))
		
	def readSensors(self):
		global g_GRobotModule
		sensors = g_GRobotModule.readSensors(byref(self))
		return {"left": 1 - (sensors & 1),
		        "right": 1 - ((sensors & 2) >> 1)}

	def getServos(self):
		global g_Servos
		servos = []
		for servo in range(len(g_Servos)):
			servos.append({"name": g_Servos[servo], "angle":self.servos[servo].angle})
		return servos

	def setServoValue(self, servo, value):
		global g_GRobotModule
		global g_ServoMap

		if servo in g_ServoMap:
			servo = g_ServoMap[servo]
		else:
			servo = int(servo)

		g_GRobotModule.setServoValue(byref(self), servo, int(value))

	def setServoAngle(self, servo, angle):
		global g_GRobotModule
		global g_ServoMap

		if servo in g_ServoMap:
			servo = g_ServoMap[servo]
		else:
			servo = int(servo)

		g_GRobotModule.setServoAngle(byref(self), servo, int(angle))

	def showDepth(self):
		g_GRobotModule.depth(byref(self))
		return self.picture.getJPG()

	def scan(self):
		g_GRobotModule.scan(byref(self))
		return self.picture.getJPG()

def init():
	global g_GRobotModule
	global g_GRobot
	global g_Servos
	global g_ServoMap;
	g_GRobotModule = cdll.LoadLibrary('./libgrobot.so')

	g_GRobotModule.readSensors.argtypes = [POINTER(TGRobot)]
	g_GRobotModule.readSensors.restype = c_int
	g_GRobotModule.setServoValue.argtypes = [POINTER(TGRobot), c_int, c_int]
	g_GRobotModule.setServoAngle.argtypes = [POINTER(TGRobot), c_int, c_int]
	g_GRobotModule.depth.argtypes = [POINTER(TGRobot)]
	g_GRobotModule.scan.argtypes = [POINTER(TGRobot)]

	for servo in range(len(g_Servos)):
		g_ServoMap[g_Servos[servo]] = servo

	return TGRobot()
