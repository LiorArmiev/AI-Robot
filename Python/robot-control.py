import numpy as np
import cv2
import pickle
import os
import io
import serial
import time
from azure.cognitiveservices.vision.face import FaceClient
from msrest.authentication import CognitiveServicesCredentials
from azure.cognitiveservices.vision.computervision import ComputerVisionClient
from azure.cognitiveservices.vision.computervision.models import VisualFeatureTypes
#from azure.cognitiveservices.vision.customvision.prediction import CustomVisionPredictionClient
arduino = serial.Serial('COM4', 9600, timeout=.1)

facekey = "yourfacekey"
endpoint = "https://xxxxxx.api.cognitive.microsoft.com/"
facecred = CognitiveServicesCredentials(facekey)
client = FaceClient(endpoint,facecred)

cvkey = "yourvisionkey"
cvcecred = CognitiveServicesCredentials(cvkey)
cvclient = ComputerVisionClient(endpoint, cvcecred)

#cuvkey = "youtcustomvisionkey"
#cuvcecred = CognitiveServicesCredentials(cuvkey)
#cuvclient = CustomVisionPredictionClient(cuvkey,endpoint)

face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read("trainner.yml")

Happinessprobability =""
playhappy = False
playsad = False

lables = {}
with open("labels.pickle", 'rb') as f:
    org_lables = pickle.load(f)
    lables = {v:k for k,v in org_lables.items()}

cap = cv2.VideoCapture(0)

visionChecked = False
iteration = 1
while(True):
    facefound = False
    ret, frame = cap.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.5, minNeighbors=5)

    if faces is None:
        playhappy = False
        playsad = False

    for (x,y,h,w) in faces:
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = frame[y:y+h, x:x+w]

        id_, conf = recognizer.predict(roi_gray)
        if conf>=30:
            font = cv2.FONT_HERSHEY_SIMPLEX
            name = lables[id_]
            color = (255,0,0)
            storke = 2
            cv2.putText(frame,name,(x+w+2,y+h),font,1,color,storke,cv2.LINE_AA)
            facefound = True

        color = (255,0,0)
        stroke = 2
        end_cord_x = x+ w
        end_cord_y = y+ h
        cv2.rectangle(frame, (x,y), (end_cord_x,end_cord_y), color, stroke)
        
        if iteration % 40 == 0:
            crop_face = frame[y:y+h, x:x+w]
            ret,buf = cv2.imencode('.jpg', crop_face)
            stream = io.BytesIO(buf)

            detected_faces = client.face.detect_with_stream(stream, return_face_id=True, return_face_attributes=['age','gender','emotion'])
            
            for detected_face in detected_faces:
                Happinessprobability = str(detected_face.face_attributes.emotion.happiness)
                Anger =str(detected_face.face_attributes.emotion.anger)
                print('anger:{}, '.format(detected_face.face_attributes.emotion.anger),end = '')
                print('happiness:{}, '.format(detected_face.face_attributes.emotion.happiness),end = '')
                print('contempt:{}, '.format(detected_face.face_attributes.emotion.contempt),end = '')
                print('disgust:{}, '.format(detected_face.face_attributes.emotion.disgust),end = '')
                print('fear:{}, '.format(detected_face.face_attributes.emotion.fear),end = '')
                print('neutral:{}, '.format(detected_face.face_attributes.emotion.neutral),end = '')
                print('sadness:{}, '.format(detected_face.face_attributes.emotion.sadness),end = '')
                print('surprise:{}'.format(detected_face.face_attributes.emotion.surprise))
    
    if iteration % 70 == 0:
        retIMG,bufIMG = cv2.imencode('.jpg', frame)
        streamIMG = io.BytesIO(bufIMG)
        image_analysis = cvclient.analyze_image_in_stream(streamIMG,visual_features=[VisualFeatureTypes.tags])
        #customvision_analysis = cuvclient.classify_image("ecb8eaa5-89f3-4666-a8e9-7f340d055b82","Iteration2",frame)
        print("Found this information in frame:")        
        for tag in image_analysis.tags:
            print('{},'.format(tag.name),end = '')
            if "banana" == tag.name:
                arduino.write(b"B\r\n")
                print("banana")
            if "darth vader" == tag.name:
                arduino.write(b"V\r\n")
                print("darth vader")
            if "glasses" == tag.name or "goggles" == tag.name:
                arduino.write(b"A\r\n")
                print("Terminator")
        print()
    iteration += 1
    
    if(Happinessprobability!="" and facefound):
        font = cv2.FONT_HERSHEY_SIMPLEX
        Happiness = float(Happinessprobability)
        Angers = float(Anger)
        if Happiness>0.90:
            color = (0,255,0)
            name = 'Happy={}'.format(Happiness)
            if (playhappy==False):
                arduino.write(b"L\r\n")
                playhappy = True
        elif Happiness<=0.90 and Happiness>0.0:
            color = (255,0,0)
            name = 'Neutral={}'.format(detected_face.face_attributes.emotion.neutral)
        elif Happiness<=0.15 and Angers>0.05:
            color = (0,0,255)
            name = 'Angre={}'.format(Anger)
            if (playsad==False):
                arduino.write(b"Q 14\r\n")
                playsad = True
        else:
            name = 'Neutral={}'.format(detected_face.face_attributes.emotion.neutral)
        storke = 2
        cv2.putText(frame,name,(x,y-5),font,1,color,storke,cv2.LINE_AA)

    cv2.imshow('frame',frame)
    if cv2.waitKey(20) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()