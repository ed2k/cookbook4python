from waveapi import events
from waveapi import model
from waveapi import robot
from waveapi import document
import logging

def OnParticipantsChanged(properties, context):
  """Invoked when any participants have been added/removed."""
  added = properties['participantsAdded']
  for p in added:
    Notify(context)

def OnRobotAdded(properties, context):
  """Invoked when the robot has been added."""
  logging.debug('Bot added to a wave')
  root_wavelet = context.GetRootWavelet()
  root_wavelet.CreateBlip().GetDocument().SetText("I'm alive!")

def Notify(context):
  root_wavelet = context.GetRootWavelet()
  root_wavelet.CreateBlip().GetDocument().SetText("Hi everybody!")

def OnBlipSubmitted(properties, context):
  blip = context.GetBlipById(properties['blipId'])
  doc = blip.GetDocument()
  img = document.Image(url='http://www.sk1project.org/images/favicon.png',caption='test')
  doc.InsertElement(0,img)
  doc.InsertText(0,'debug:')

if __name__ == '__main__':
  myRobot = robot.Robot('test8898', 
      image_url='http://www.sk1project.org/images/favicon.png',
      version='1',
      profile_url='http://test8898.appspot.com/')
  myRobot.RegisterHandler(events.BLIP_SUBMITTED, OnBlipSubmitted)
  myRobot.RegisterHandler(events.WAVELET_PARTICIPANTS_CHANGED, OnParticipantsChanged)
  myRobot.RegisterHandler(events.WAVELET_SELF_ADDED, OnRobotAdded)
  myRobot.Run()
  
  