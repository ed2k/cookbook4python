#!/usr/bin/env python
# -*- coding: iso-8859-15 -*- 

import dbus, sys

class MainWindow:
	# get new command from entry; run; clear
	def send_command(self, widget, entry):
		entry_text = entry.get_text()
		if entry_text == "":
			return
		entry.set_text("")
		answer = self.api_object.send_message(entry_text)	 

	def write_textbuffer(self, message):
		enditer = self.textbuffer.get_end_iter()
		mark = self.textbuffer.create_mark(None, enditer, False)
		self.textbuffer.insert(enditer, message + '\n')
		self.textview.scroll_to_mark(mark, 0, True, 0.0, 1.0)
		self.textbuffer.delete_mark(mark)

	def __init__(self):
		
		#objects
		self.api_object = Api_obj(self)
		self.callback_obj = Callback_obj(dbus.Service("com.Skype.API", dbus.SystemBus()), self)

class Callback_obj(dbus.Object):
	def __init__(self, service, mw):
		self.mainwindow = mw
		dbus.Object.__init__(self, "/com/Skype/Client", service, [self.Notify])
	
	# Skype -> Client messages
	def Notify(self, message, message_text):
		#self.mainwindow.write_textbuffer(message+':'+message_text)
                self.mainwindow.write_textbuffer('N:'+message_text)
                print message

class Api_obj:
	def __init__(self, mw):
		remote_bus = dbus.SystemBus()
		
		system_service_list = remote_bus.get_service('org.freedesktop.DBus').get_object('/org/freedesktop/DBus', 'org.freedesktop.DBus').ListServices()
                print system_service_list
		skype_api_found = 0

		for service in system_service_list:
			if service=='com.Skype.API':
				skype_api_found = 1
				break

		if not skype_api_found:
			print('No running API-capable Skype found')
                        return

		skype_service = remote_bus.get_service('com.Skype.API')
		self.skype_api_object = skype_service.get_object('/com/Skype','com.Skype.API')

		answer = self.send_message('NAME SkypeApiPythonTestClient')
		if answer != 'OK':
			sys.exit('Could not bind to Skype client')

		answer = self.send_message('PROTOCOL 5')
		if answer != 'PROTOCOL 5':
			sys.exit('This test program only supports Skype API protocol version 1')

	# Client -> Skype
	def send_message(self, message):		
		self.mainwindow.write_textbuffer(message)
		answer = self.skype_api_object.Invoke(message)
		self.mainwindow.write_textbuffer(answer)
		return answer

class ModeSetObject(dbus.Object):
    def __init__(self, bus_name, object_path='/org/freedesktop/ModeSet'):
        dbus.Object.__init__(self, object_path, bus_name)

    @dbus.method('org.freedesktop.ModeSetIface')
    def getmodelist(self):
        print 'getmodelist'

    @dbus.method('org.freedesktop.ModeSetIface')
    def setmode(self, modenum):
        return 'setmodelist'



system_bus = dbus.SystemBus()
bus_name = dbus.Service('org.freedesktop.ModeSet', bus=system_bus)

object = ModeSetObject(bus_name)

MainWindow()


